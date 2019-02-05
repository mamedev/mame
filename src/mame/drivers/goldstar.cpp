// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Vas Crabb
/***************************************************************************

  Golden Star
  Cherry Master
  Lucky 8 Lines
  Amcoe CGA games

  Golden Star and Cherry Master seem to be almost the same thing, running on
  different hardware.  There are also various bootlegs / hacks, it isn't clear
  exactly what hardware each runs on, some appear to have no OKI for example.

  Driver by Mirko Buffoni.
  Additional Work: David Haywood & Roberto Fresca.

  The vast majority of the sets in here are probably bootlegs and hacks
  hence the slightly different PCBs, rom layouts, slightly hacked program roms
  etc.

****************************************************************************

  Game notes:
  -----------

  * Wing Game Boards & Games (Originals):

  Various types
    - older pcbs can be green, blue or black
    - newer pcbs are green
    - might also be short & long types of each

  Sub-boards are connected into the Z80 socket and all appear to be bootleg
  upgrades/conversions/hacks.  Are any of them legit?

  W-4   New Lucky 8 Lines
        Bingo
  F-5   New Lucky 8 Lines w/ Witch Bonus - bootleg/hack??
  W-6   Fever Chance
  W-7   Skill Chance
  W-8   Bonus Chance
  W-11  Lucky Star


  * New Lucky 8 Lines:

  Sometimes the game boots with a "Coin Jam" message. Just reset the game to normalize.
  There are 2 sets of controls. Press the BIG key to switch between them.

  Press 9 to enter settings, press START to exit.
  Press 0 to enter stats, press START to exit.
  Keeping pressed 9 + 0 + RESET (F3), will enter the test mode. Press RESET to exit.

  New Lucky 8 Lines has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.


  * New Lucky 8 Lines / New Super 8 Lines:

  This set has a regular New Lucky 8 Lines, but allowing bets up to 64.
  It has different settings/parameters, and additional graphics for a game
  called New Super 8 Lines. There are basic reels tiles with a semi-naked woman,
  a sort of Super Mario character from Nintendo, clouds and stars...

  Still can't find how to switch between games.


  * New Lucky 8 Lines / New Super 8 Lines (Witch Bonus):

  Same as above, but allowing bets up to 32 credits.
  It also has different settings/parameters, and a rare feature: the 'Witch Bonus'
  (from Witch Card poker game) is present in the double-up.


  * Cherry Bonus III:

  If a hopper status error appear when the player try to take score,
  pressing Key Out (W) will discharge the credits won.

  Cherry Bonus III has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.

  Controls Set2 is using reels stop buttons from Controls Set1.


  * Cherry Master V4 (set 2)

  This set is supposed to be a kind of "stealth".
  The game is hidden into a Tetris game and could be triggered/switched
  in some way. Seems that it was designed for locations/countries where
  gambling games are/were not allowed.

  The game is booting as Cherry Master V4 instead of Tetris ATM...

  Even when the gambling game is working properly, the game is flagged
  as NOT_WORKING till can figure out how can switch between games.


  * Cherry Master '99 (V9B.00)

  Inputs are hooked up as DIP switches for testing purposes.  You can access
  some kind of palette test by momentarily flipping input 2-8 to On position.
  You can access the input port test by flipping input 2-7 to On position while
  the game boots (like other Cherry Master games).  All inputs are hooked up
  correctly.

  When not in the input test, the game appears to be stuck in a loop resetting
  itself after initialising NVRAM.


  * Cherry Master '99 (V9B.00 bootleg / hack)

  Appears to be a bootleg/hack of the other Dyna set - NVRAM initialisation has
  been changed from "DYNA QL-1 V9B.00" to "     QQ-1 V9B.0", very few other
  program ROM changes besides hacking out the Dyna name everywhere.


  * Kkoj Noli

  kkuj nol-i / kkoj noli (better romanization).

  kkuj = stab/kill
  nol-i = bees

  The little red box at bottom of the title translates as "South Korea"

  Seems to be a hack of Lucky 8 Lines.

  - Child'ish graphics.
  - For Amusement only... There is no payout/keyout line accessed.
  - No stats or service mode.
  - No NVRAM.
  - Two sets of player's controls, as lucky8.
  - No ay8910, so no extra ports.
  - Only 1 DIP switches bank.

  Nominated for the *WORST* hacked gambling game EVER!


  * unkch sets

  In unkch1/unkch2 the payout rate is set with a combination of DSW1-3 (Punti)
  and DSW3-3 (Gettoni/Ticket).  If Punti is set to Ticket, the payout rate is
  the second number of the Gettoni/Ticket setting (100 or 200).  If Punti is set
  to Gettoni, the payout rate is the first number of the Gettoni/Ticket setting
  (10 or 20).  If your points/credits aren't a multiple of the payout rate, you
  lose the remainder.  If you hit Key Out when your points/credits are less than
  100, you get nothing at all.  If Gettoni/Ticket is set to 20/200 and you hit
  Key Out when credits/points are at least 100 but less than 200, tickets will
  be dispensed continuously until you insert another coin - game bug or MAME
  bug?

  Payout rate in unkch3 seems to be set with DSW1-3 (Punti) directly.  This game
  also seems to be able to easily get into a state where tickets are dispensed
  continuously.  Maybe there's something more complicated about the ticket
  dispenser hookup that we're missing?

  In unkch4 the payout rate is set with DSW1-3 (Punti) - 100 for Ticket and 10
  for Gettoni.  It's also nice enough to let you keep the remainder if you hit
  Key Out when your credits/points aren't a multiple of 100.  This is the only
  set that doesn't have issues with dispensing tickets continuously

  unkch3 has a handy input test mode.  To access it, first enable it with DSW4-5,
  then hold the Settings button (9) during boot.


  * Crazy Bonus (crazybon):

  Appears to be from a bootleg conversion set for Poker Master (pkrmast).  There
  is another undumped bootleg conversion set advertised that displays Spirit or
  Dyna copyright depending on DIP settings and has both poker and slots games (the
  set in MAME displays "Crazy Co." copyright and only has a slots game).

  This is a stealth set that hides behind a fake Windows ME desktop if DSW2-6 is
  off.  Push Start followed by Bet five time to access the game.  It will return
  to the desktop after the game is over.  Colours currently appear to be bad on
  the desktop screen.  DSW3-8 disables the button sequence for accessing the game.

  Judging from the contents of the graphics ROMs and the Stats screens, there's a
  poker game buried in there, but there's apparently no way to access it.

  Hold Settings button (9) during boot to access switch test.
  Hold Stats button (0) during boot to access palette test.


  * Super Nove (Playmark) (super9)

  This game has similar memory map than Golden Star. The program writes to init
  the reels RAM/palette (bp C11A), then transfer the control to PC 253h where
  starts to write to some NVRAM chunks. Unfortunatelly at PC 2DBh there is a call
  to 0C33h, where there are only ASCII strings instead of subroutines.
  Also there are some other calls to the same range, that also lack of code.


  * Tetris + Cherry Master (+K, Canada Version, encrypted) (cmtetrsb)

  Start the game and you can find some garbage due to wrong graphics banks.
  Press the key "insert" to throttle the game. Keep the key pressed till
  you can see what seems the attract working (still with wrong graphics).

  Seems to be sooo slow.... (interrupts?)


  * Bingo (Wing)

  It has a different machine driver to support the different pos of gfx
  layers. I strogly suspect there is a register to adjust the layer position.


***************************************************************************/


#include "emu.h"
#include "includes/goldstar.h"

#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "video/ramdac.h"

#include "screen.h"
#include "speaker.h"

#include "bingowng.lh"
#include "cherryb3.lh"
#include "chrygld.lh"
#include "cmaster.lh"
#include "cmasterb.lh"
#include "cmasterc.lh"
#include "cmezspin.lh"
#include "cmpacman.lh"
#include "cmv4.lh"
#include "crazybon.lh"
#include "goldstar.lh"
#include "lucky8.lh"
#include "lucky8p1.lh"
#include "nfb96.lh"
#include "nfb96tx.lh"
#include "pokonl97.lh"
#include "roypok96.lh"
#include "skill98.lh"
#include "tonypok.lh"
#include "unkch.lh"

namespace {

constexpr XTAL MASTER_CLOCK = 12_MHz_XTAL;
constexpr XTAL CPU_CLOCK    = MASTER_CLOCK / 4;
constexpr XTAL PSG_CLOCK    = MASTER_CLOCK / 4;
constexpr XTAL AY_CLOCK     = MASTER_CLOCK / 8;
#define OKI_CLOCK       1056000      /* unverified resonator */

}



WRITE8_MEMBER(goldstar_state::protection_w)
{
	if (data == 0x2a)
		m_dataoffset = 0;
}

READ8_MEMBER(goldstar_state::protection_r)
{
	static const int data[4] = { 0x47, 0x4f, 0x4c, 0x44 };

	m_dataoffset %= 4;
	return data[m_dataoffset++];
}

WRITE8_MEMBER(goldstar_state::p1_lamps_w)
{
/*  bits
  7654 3210     goldstar                            crazybon                ncb3/cb3a               lucky8/bingowng
  ---- ---x     Bet Red / Card 2                                            Stop 2 / Big
  ---- --x-     Stop 3 / Small / Info / Card 1      Start                   Blue Bet / Double       D-UP
  ---- -x--     Bet Blue / Double Up / Card 3                               Stop 1/Take             TAKE
  ---- x---     Stop 1 / Take                       Bet                     Red Bet                 BET
  ---x ----     Stop 2 / Big / Bonus                Stop All / Take Score   Stop 3 / Small / Info   INFO
  --x- ----     Start / Stop All / Card 4           Double Up               Start / Stop All        START
  -x-- ----                                         Small / Info
  x--- ----                                         Big

  7654 3210     cm/cmaster  cmpacman/cmtetris   tonypok     schery97        pokonl97        match98
  ---- ---x                                                 stop/big        bet 10/big      hit/stop
  ---- --x-     d-up        d-up                big/small   d-up            d-up
  ---- -x--     take        take/stop           take/d-up   take/select     take/select     take
  ---- x---     bet         bet                 bet         bet             bet 1           bet
  ---x ----     info        info                            small           small/end
  --x- ----     start       start               deal        start           start           start
  -x-- ----                                     hold
  x--- ----

  all cm/cmaster use the same scheme
  cmv4, cmv801 and crazybon don't light the Take button when it's available for hold pair
  tonypok uses lamps to indicate current button functions rather than active buttons
  skill98 is like schery97 but doesn't activate bit 0 for stop
  nfb96, roypok96 and nc96 sets are like schery97 but they don't activate bit 2 for select
*/
	m_lamps[0] = BIT(data, 0);
	m_lamps[1] = BIT(data, 1);
	m_lamps[2] = BIT(data, 2);
	m_lamps[3] = BIT(data, 3);
	m_lamps[4] = BIT(data, 4);
	m_lamps[5] = BIT(data, 5);
	m_lamps[6] = BIT(data, 6);
	m_lamps[7] = BIT(data, 7);

//  popmessage("p1 lamps: %02X", data);
}

WRITE8_MEMBER(goldstar_state::p2_lamps_w)
{
	m_lamps[8 + 0] = BIT(data, 0);
	m_lamps[8 + 1] = BIT(data, 1);
	m_lamps[8 + 2] = BIT(data, 2);
	m_lamps[8 + 3] = BIT(data, 3);
	m_lamps[8 + 4] = BIT(data, 4);
	m_lamps[8 + 5] = BIT(data, 5);
	m_lamps[8 + 6] = BIT(data, 6);
	m_lamps[8 + 7] = BIT(data, 7);

//  popmessage("p2 lamps: %02X", data);
}


void goldstar_state::goldstar_map(address_map &map)
{
	map(0x0000, 0xb7ff).rom();
	map(0xb800, 0xbfff).ram().share("nvram");
	map(0xc000, 0xc7ff).rom();
	map(0xc800, 0xcfff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xd000, 0xd7ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0xd800, 0xd9ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xe000, 0xe1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xe800, 0xe9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf040, 0xf07f).ram().share("reel1_scroll");
	map(0xf080, 0xf0bf).ram().share("reel2_scroll");
	map(0xf0c0, 0xf0ff).ram().share("reel3_scroll");

	map(0xf800, 0xf800).portr("IN0");
	map(0xf801, 0xf801).portr("IN1");    /* Test Mode */
	map(0xf802, 0xf802).portr("DSW1");
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	map(0xf805, 0xf805).portr("DSW4");   /* DSW 4 (also appears in 8910 port) */
	map(0xf806, 0xf806).portr("DSW7");   /* (don't know to which one of the */
	map(0xf810, 0xf810).portr("UNK1");
	map(0xf811, 0xf811).portr("UNK2");
	map(0xf820, 0xf820).portr("DSW2");
	map(0xf830, 0xf830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xf840, 0xf840).w("aysnd", FUNC(ay8910_device::address_w));
	map(0xf900, 0xf900).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xfa00, 0xfa00).w(FUNC(goldstar_state::goldstar_fa00_w));
	map(0xfb00, 0xfb00).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfd00, 0xfdff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xfe00, 0xfe00).rw(FUNC(goldstar_state::protection_r), FUNC(goldstar_state::protection_w));
}

void goldstar_state::goldstar_readport(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).portr("DSW6");
}


void sanghopm_state::star100_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();

	map(0xc800, 0xcfff).ram().w(FUNC(sanghopm_state::fg_vidram_w)).share("fg_vidram");    // videoram 1
	map(0xd000, 0xd7ff).ram().w(FUNC(sanghopm_state::fg_atrram_w)).share("fg_atrram");    // atrram 1

	map(0xd800, 0xd83f).ram().share("reel1_scroll");
	map(0xd840, 0xd9ff).ram();
	map(0xda00, 0xda3f).ram().share("reel2_scroll");
	map(0xda40, 0xdbff).ram();
	map(0xdc00, 0xdc3f).ram().share("reel3_scroll");
	map(0xdc40, 0xdfff).ram();

	map(0xe000, 0xe1ff).ram().w(FUNC(sanghopm_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xe200, 0xe3ff).ram().w(FUNC(sanghopm_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xe400, 0xe5ff).ram().w(FUNC(sanghopm_state::goldstar_reel3_ram_w)).share("reel3_ram");

	map(0xe600, 0xe7ff).ram().w(FUNC(sanghopm_state::bg_vidram_w)).share("bg_vidram");    // videoram 2

	map(0xe800, 0xe9ff).ram().w(FUNC(sanghopm_state::reel1_attrram_w)).share("reel1_attrram");
	map(0xea00, 0xebff).ram().w(FUNC(sanghopm_state::reel2_attrram_w)).share("reel2_attrram");
	map(0xec00, 0xedff).ram().w(FUNC(sanghopm_state::reel3_attrram_w)).share("reel3_attrram");

	map(0xee00, 0xefff).ram().w(FUNC(sanghopm_state::bg_atrram_w)).share("bg_atrram");    // atrram 2

	map(0xf000, 0xf7ff).ram().share("nvram");
	map(0xf800, 0xffff).ram();

}


WRITE8_MEMBER(sanghopm_state::coincount_w)
{
/*
  7654 3210
  ---- ---x  Coin Out counter.
  ---- x---  Coin A counter..
  ---x ----  Coin B counter.
  --x- ----  Key In counter.
  -x-- ----  Coin C counter.
  x--- -xx-  Unknown.

*/
	machine().bookkeeping().coin_counter_w(0, data & 0x08);  /* counter1 coin a */
	machine().bookkeeping().coin_counter_w(1, data & 0x10);  /* counter2 coin b */
	machine().bookkeeping().coin_counter_w(2, data & 0x20);  /* counter3 key in */
	machine().bookkeeping().coin_counter_w(3, data & 0x40);  /* counter4 coin c */
	machine().bookkeeping().coin_counter_w(4, data & 0x01);  /* counter5 payout */
}

WRITE8_MEMBER(sanghopm_state::enable_w)
{
	m_enable_reg = data;
}

void sanghopm_state::star100_readport(address_map &map)
{
	map.global_mask(0xff);

	map(0x08, 0x08).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w("aysnd", FUNC(ay8910_device::address_w));

	map(0x10, 0x10).portr("IN0");
	map(0x11, 0x11).portr("IN1");
	map(0x12, 0x12).portr("IN2");
	map(0x13, 0x13).portr("IN3");
	map(0x14, 0x14).portr("DSW1");

	map(0x1c, 0x1c).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x1d, 0x1d).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x1e, 0x1e).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x20, 0x20).portr("DSW4-0");     // the first 4 bits map to DSW4 1 to 4.
	map(0x21, 0x21).portr("DSW4-1");     // the first 4 bits map to DSW4 5 to 8.

	map(0x24, 0x24).w(FUNC(sanghopm_state::coincount_w));      // coin counters.

	map(0x25, 0x25).portr("DSW2");
	map(0x26, 0x26).portr("DSW3");

	map(0xe0, 0xe0).nopw();                // Writing 0's and 1's constantly.  Watchdog feeder?
	map(0xe1, 0xe1).w(FUNC(sanghopm_state::enable_w));         // enable/disable reels register.

}

/*
  08:  W (3F)   AY8910 data
  0C:  W (07)   AY8910 address

  10: R     Input #0
  11: R     Input #1
  12: R     Input #2
  13: R     Input #3

  14: R     DSW #1

  17:  W

  1C:  W    \ these looks like RAMDAC commands. After write to 1C, there are 3 writes to 1D.
  1D:  W    / bp: 6c5b.
  1E:  W (FF) --> seems the RAMDAC mask.

  20: R    DSW #4-0
  21: R    DSW #4-1
  22:  W
  24:  W
  25: R    DSW #2
  26: R    DSW #3

  2F:  W (9C)

  E0:  W
  E1:  W    Reels enable/disable register
 */

void goldstar_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

/*
  RAMDAC written commands:

  1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D
  -------------       -------------       -------------       -------------
  00   00 00 00       10   00 00 00       20   00 00 00       30   00 00 00
  01   E8 18 06       11   E8 18 06       21   E8 18 06       31   E8 18 06
  02   FC EA 00       12   FC EA 00       22   FC EA 00       32   FC EA 00
  03   FF FF 00       13   FF FF 00       23   FF FF 00       33   FF FF 00
  04   FF 00 00       14   FF 00 00       24   FF 00 00       34   FF 00 00
  05   00 00 FF       15   00 00 FF       25   00 00 FF       35   00 00 FF
  06   00 E6 00       16   00 E6 00       26   00 E6 00       36   00 E6 00
  07   01 F0 02       17   01 F0 02       27   01 F0 02       37   01 F0 02
  08   EF FF E8       18   EF FF E8       28   EF FF E8       38   EF FF E8
  09   12 08 F2       19   12 08 F2       29   12 08 F2       39   12 08 F2
  0A   1A 12 FF       1A   1A 12 FF       2A   1A 12 FF       3A   1A 12 FF
  0B   1F 1F F9       1B   1F 1F F9       2B   1F 1F F9       3B   1F 1F F9
  0C   F9 F9 F9       1C   F9 F9 F9       2C   F9 F9 F9       3C   F9 F9 F9
  0D   EF 18 00       1D   EF 18 00       2D   EF 18 00       3D   EF 18 00
  0E   F0 F0 F0       1E   F0 F0 F0       2E   F0 F0 F0       3E   F0 F0 F0
  0F   FF FF FF       1F   FF 00 00       2F   00 FF FF       3F   00 FF 00


  1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D
  -------------       -------------       -------------       -------------
  40   00 00 00       50   00 00 00       60   00 00 00       70   00 00 00
  41   E8 18 06       51   E8 18 06       61   E8 18 06       71   E8 18 06
  42   FC EA 00       52   FC EA 00       62   FC EA 00       72   FC EA 00
  43   FF FF 00       53   FF FF 00       63   FF FF 00       73   FF FF 00
  44   FF 00 00       54   FF 00 00       64   FF 00 00       74   FF 00 00
  45   00 00 FF       55   00 00 FF       65   00 00 FF       75   00 00 FF
  46   00 E6 00       56   00 E6 00       66   00 E6 00       76   00 E6 00
  47   01 F0 02       57   01 F0 02       67   01 F0 02       77   01 F0 02
  48   EF FF E8       58   EF FF E8       68   EF FF E8       78   EF FF E8
  49   12 08 F2       59   12 08 F2       69   12 08 F2       79   12 08 F2
  4A   1A 12 FF       5A   1A 12 FF       6A   1A 12 FF       7A   1A 12 FF
  4B   1F 1F F9       5B   1F 1F F9       6B   1F 1F F9       7B   1F 1F F9
  4C   F9 F9 F9       5C   F9 F9 F9       6C   F9 F9 F9       7C   F9 F9 F9
  4D   EF 18 00       5D   EF 18 00       6D   EF 18 00       7D   EF 18 00
  4E   F0 F0 F0       5E   01 EC FF       6E   00 00 00       7E   00 00 00
  4F   FF FF 00       5F   00 00 00       6F   01 EC FF       7F   00 00 00


  1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D
  -------------       -------------       -------------       -------------
  80   28 28 28       90   28 28 28       A0   28 28 28       B0   28 28 28
  81   FF ED E3       91   0B 00 00       A1   18 11 00       B1   02 16 00
  82   FF F7 1E       92   13 00 00       A2   00 E1 00       B2   02 E1 00
  83   FF F8 18       93   1C 00 00       A3   00 E8 00       B3   01 ED 02
  84   14 0B 0B       94   E5 00 00       A4   13 F0 00       B4   02 FA 09
  85   1F 10 0A       95   ED 00 00       A5   FF F7 1E       B5   F1 1A 00
  86   E3 13 08       96   F6 00 00       A6   FF F0 EE       B6   F1 EA 00
  87   E8 18 06       97   FF 05 0D       A7   EF 18 00       B7   1F 00 00
  88   EC 1E 03       98   FF 0C 13       A8   F4 1B 00       B8   F2 00 00
  89   F0 E3 02       99   FF 13 19       A9   FD 1F 00       B9   FF 00 00
  8A   F5 E9 01       9A   FF 1A 1F       AA   FF E3 00       BA   FF 0C 02
  8B   FA EF 01       9B   FF E2 E6       AB   FF EA 00       BB   FF 18 06
  8C   FF F6 00       9C   FF E9 EC       AC   FF F1 00       BC   FF E3 0A
  8D   FF FF 00       9D   FF F0 F2       AD   FF F8 00       BD   FF EC 19
  8E   FF FF 1F       9E   FF F7 F8       AE   FF FF 00       BE   FF F4 1F
  8F   FF FF FF       9F   FF FF FF       AF   FF FF 1F       BF   FF FF FF


  1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D       1C   1D 1D 1D
  -------------       -------------       -------------       -------------
  C0   28 28 28       D0   28 28 28       E0   28 28 28       F0   12 12 E0
  C1   F1 E6 00       D1   00 00 E4       E1   00 05 00       F1   1B 00 00
  C2   F8 F1 00       D2   00 00 FF       E2   00 0A 00       F2   E0 10 04
  C3   FF FF 00       D3   00 13 FF       E3   00 10 00       F3   E9 14 05
  C4   14 07 E7       D4   00 1A FF       E4   00 15 00       F4   EF 1E 12
  C5   15 0C EF       D5   00 E8 FF       E5   00 1B 00       F5   EC E3 1A
  C6   1A 12 FA       D6   16 0B 00       E6   00 E0 00       F6   EE 05 04
  C7   1B 16 FA       D7   19 0F 00       E7   00 E6 00       F7   F4 05 04
  C8   1D 1A FB       D8   1D 14 00       E8   04 EC 05       F8   FE 04 04
  C9   1F 1E FC       D9   E5 1A 00       E9   0C F2 0E       F9   F6 E4 15
  CA   E2 E2 FC       DA   EE E0 00       EA   15 F8 18       FA   F8 EC E0
  CB   E6 E7 FD       DB   F6 E6 00       EB   E0 FF E3       FB   FD F0 E4
  CC   EA EC FE       DC   FF ED 00       EC   FD 0C 02       FC   FF F6 E7
  CD   EF F1 FF       DD   FF F6 00       ED   FD 12 0D       FD   FF FA EA
  CE   F6 F8 FF       DE   FF FF 00       EE   FE 1A 18       FE   FF FF F2
  CF   FF FF FF       DF   FF FF FF       EF   FF E4 E4       FF   FF FF FF


  And set again....

  1C   1D 1D 1D       1C   1D 1D 1D
  -------------       -------------
  70   00 00 00       F0   12 12 E0
  71   18 0C F6       F1   1B 00 00
  72   1B 13 0B       F2   E0 10 04
  73   E1 15 1E       F3   E9 14 05
  74   F3 E9 E4       F4   EF 1E 12
  75   F3 E7 00       F5   EC E3 1A
  76   FE 17 E3       F6   EE 05 04
  77   FE 0C F1       F7   F4 05 04
  78   FD E3 13       F8   FE 04 04
  79   FE ED 05       F9   F6 E4 15
  7A   FF F5 03       FA   F8 EC E0
  7B   FF FB 14       FB   FD F0 E4
  7C   FC FB FA       FC   FF F6 E7
  7D   FF FC E0       FD   FF FA EA
  7E   FE FE FC       FE   FF FF F2
  7F   FF 00 FF       FF   FF FF FF

*/



WRITE8_MEMBER(goldstar_state::ncb3_port81_w)
{
//  if (data!=0x00)
//      popmessage("ncb3_port81_w %02x\n",data);
}


void cb3_state::ncb3_map(address_map &map)
{
	map(0x0000, 0xb7ff).rom();
	map(0xb800, 0xbfff).ram().share("nvram");
	map(0xc000, 0xc7ff).rom();
	map(0xc800, 0xcfff).ram().w(FUNC(cb3_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xd000, 0xd7ff).ram().w(FUNC(cb3_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0xd800, 0xd9ff).ram().w(FUNC(cb3_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xe000, 0xe1ff).ram().w(FUNC(cb3_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xe800, 0xe9ff).ram().w(FUNC(cb3_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf040, 0xf07f).ram().share("reel1_scroll");
	map(0xf080, 0xf0bf).ram().share("reel2_scroll");
	map(0xf100, 0xf17f).ram().share("reel3_scroll"); // moved compared to goldstar

	map(0xf800, 0xf803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xf810, 0xf813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xf820, 0xf823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */
	map(0xf822, 0xf822).w(FUNC(cb3_state::goldstar_fa00_w)); // hack (connected to ppi output port?, needed for colour banking)

	map(0xf830, 0xf830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xf840, 0xf840).w("aysnd", FUNC(ay8910_device::address_w));
	map(0xf850, 0xf850).w(FUNC(cb3_state::p1_lamps_w));       /* Control Set 1 lamps */
	map(0xf860, 0xf860).w(FUNC(cb3_state::p2_lamps_w));       /* Control Set 2 lamps */
	map(0xf870, 0xf870).w("snsnd", FUNC(sn76489_device::write));    /* guess... device is initialized, but doesn't seems to be used.*/
}

void goldstar_state::ncb3_readwriteport(address_map &map)
{
	map.global_mask(0xff);
//  AM_RANGE(0x00, 0x00) AM_READ(ncb3_unkread_r)    // read from 0x00 when controls set 1 is used...
//  AM_RANGE(0x02, 0x02) AM_READ(ncb3_unkread_r)    // read from 0x02 when controls set 2 is used...
//  AM_RANGE(0x06, 0x06) AM_READ(ncb3_unkread_r)    // unknown...
//  AM_RANGE(0x08, 0x08) AM_READ(ncb3_unkread_r)    // unknown...
	map(0x10, 0x10).portr("DSW5");   /* confirmed for ncb3 */
	map(0x81, 0x81).w(FUNC(goldstar_state::ncb3_port81_w)); // ---> large writes.

}

/* ncb3 findings...

  c101-c102 = unknown writes...
  f800-f803 = 8255_1 (ctrl=9b) ; portA, B & C (input)
  f810-f813 = 8255_2 (ctrl=9b) ; portA, B & C (input)
  f820-f823 = 8255_3 (ctrl=90) ; portA (input); ports B & C (output)
  f830      = AY8910 RW
  f840      = AY8910 ctrl
  f850      = control set 1 lamps
  f860      = control set 2 lamps
  f870      = PSG (init writes)


  I/O

  00 = RW  (chrygld, ncb3 in ctrl set1)
  02 = R  (W - ncb3 in ctrl set2)
  06 = W
  08 = RW
  81 =  W

  00-0f = initial seq. writes

  Controls Set 1 = write lamps to f850, read from 0002.
  Controls Set 2 = write lamps to f860, read from 0000.

  Controls Set 2 is using reels stop buttons from Controls Set 1.
  So, seems that control set 2 was meant for non-stop reels.

*/


void goldstar_state::wcherry_map(address_map &map)
{
	map(0x0000, 0xb7ff).rom();
	map(0xb800, 0xbfff).ram().share("nvram");
	map(0xc000, 0xc7ff).rom();

	/* Video RAM and reels stuff are there just as placeholder, and obviously in wrong offset */
	map(0xc800, 0xcfff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xd000, 0xd7ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0xd800, 0xd9ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xe000, 0xe1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xe800, 0xe9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf040, 0xf07f).ram().share("reel1_scroll");
	map(0xf080, 0xf0bf).ram().share("reel2_scroll");
	map(0xf0c0, 0xf0ff).ram().share("reel3_scroll");

	/* Not really PPI's... They are emulated/simulated inside the CPLDs */
	map(0xf600, 0xf603).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xf610, 0xf613).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xf620, 0xf623).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */

	map(0xf630, 0xf630).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xf640, 0xf640).w("aysnd", FUNC(ay8910_device::address_w));
	map(0xf650, 0xf650).nopw();    // AM_WRITE(output_w)  // unknown register: 0x3e
	map(0xf660, 0xf660).nopw();    // AM_WRITE(output_w)  // unknown register: 0x3e
	map(0xf670, 0xf670).w("snsnd", FUNC(sn76489_device::write));    /* guess... device is initialized, but doesn't seems to be used.*/

	map(0xf800, 0xffff).ram();
}

void goldstar_state::wcherry_readwriteport(address_map &map)
{
	map.global_mask(0xff);
	}

/* wcherry findings...

  0000-bfff = ROM space.
  b000-b7ff = NVRAM.
  c000-c7ff = ROM space.

  f600-f603 = 8255_1 (ctrl=9b) ; portA, B & C (input)
  f610-f613 = 8255_2 (ctrl=9b) ; portA, B & C (input)
  f620-f623 = 8255_3 (ctrl=90) ; portA (input); ports B & C (output)
  f630      = AY8910 RW
  f640      = AY8910 ctrl
  f650      = Unknown. Seems a register. Writes 0x3e.
  f660      = Unknown. Seems a register. Writes 0x3e.
  f670      = PSG (init writes)


  I/O

*/


void goldstar_state::cm_map(address_map &map)
{
	map(0x0000, 0xcfff).rom().nopw();

	map(0xd000, 0xd7ff).ram().share("nvram");
	map(0xd800, 0xdfff).ram();

	map(0xe000, 0xe7ff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");

	map(0xf000, 0xf1ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xf200, 0xf3ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xf400, 0xf5ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf600, 0xf7ff).ram();

	map(0xf800, 0xf87f).ram().share("reel1_scroll");
	map(0xf880, 0xf9ff).ram();
	map(0xfa00, 0xfa7f).ram().share("reel2_scroll");
	map(0xfa80, 0xfbff).ram();
	map(0xfc00, 0xfc7f).ram().share("reel3_scroll");
	map(0xfc80, 0xffff).ram();
}


void goldstar_state::nfm_map(address_map &map)
{
	map(0x0000, 0xd7ff).rom().nopw();

	map(0xd800, 0xdfff).ram().share("nvram");

	map(0xe000, 0xe7ff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");

	map(0xf000, 0xf1ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xf200, 0xf3ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xf400, 0xf5ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf600, 0xf7ff).ram();

	map(0xf800, 0xf87f).ram().share("reel1_scroll");
	map(0xf880, 0xf9ff).ram();
	map(0xfa00, 0xfa7f).ram().share("reel2_scroll");
	map(0xfa80, 0xfbff).ram();
	map(0xfc00, 0xfc7f).ram().share("reel3_scroll");
	map(0xfc80, 0xffff).ram();
}



WRITE8_MEMBER(goldstar_state::cm_coincount_w)
{
/*  bits
  7654 3210
  ---- ---x  Coin Out counter
  ---- x---  Coin D counter
  ---x ----  Coin C counter
  --x- ----  Key In counter
  -x-- ----  Coin A counter
  x--- -xx-  unknown

  interestingly there is no counter for coin B in the cm/cmaster games
*/

	machine().bookkeeping().coin_counter_w(0, data & 0x40);  /* Counter 1 Coin A */
	machine().bookkeeping().coin_counter_w(1, data & 0x20);  /* Counter 2 Key In */
	machine().bookkeeping().coin_counter_w(2, data & 0x10);  /* Counter 3 Coin C */
	machine().bookkeeping().coin_counter_w(3, data & 0x08);  /* Counter 4 Coin D */
	machine().bookkeeping().coin_counter_w(4, data & 0x01);  /* Counter 5 Payout */

//  if (data & 0x86) // triggered by fb2010
//      popmessage("counters: %02X", data);
}

void cmaster_state::cm_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x04, 0x07).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Inputs */
	map(0x08, 0x0b).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* DIP switches */
	map(0x10, 0x10).w(FUNC(cmaster_state::outport0_w));
	map(0x11, 0x11).w(FUNC(cmaster_state::cm_coincount_w));
	map(0x12, 0x12).w(FUNC(cmaster_state::p1_lamps_w));
	map(0x13, 0x13).w(FUNC(cmaster_state::background_col_w));
	map(0x14, 0x14).w(FUNC(cmaster_state::girl_scroll_w));
}


void goldstar_state::pkrmast_portmap(address_map &map)
{
	map.global_mask(0xff);

	map(0x04, 0x04).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x08, 0x08).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w("aysnd", FUNC(ay8910_device::address_w));

	map(0x10, 0x10).portr("IN0");
	map(0x11, 0x11).portr("IN1");
	map(0x12, 0x12).portr("IN2");

	map(0x20, 0x20).portr("DSW3-0");
	map(0x21, 0x21).portr("DSW3-1");
	map(0x22, 0x22).w(FUNC(goldstar_state::p1_lamps_w));

	map(0x24, 0x24).w(FUNC(goldstar_state::cm_coincount_w));
	map(0x25, 0x25).portr("DSW1");
	map(0x26, 0x26).portr("DSW2");

	map(0xf0, 0xf0).nopw();    /* Writing 0's and 1's constantly.  Watchdog feeder? */
}


void goldstar_state::cmast91_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0x10, 0x13).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* DIP switches */
	map(0x21, 0x21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x22, 0x23).w("aysnd", FUNC(ay8910_device::data_address_w));
}


void cmaster_state::amcoe1_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x04, 0x07).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0x08, 0x0b).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* DIP switches */
	map(0x10, 0x10).w(FUNC(cmaster_state::outport0_w));
	map(0x11, 0x11).w(FUNC(cmaster_state::cm_coincount_w));
	map(0x12, 0x12).w(FUNC(cmaster_state::p1_lamps_w));
	map(0x13, 0x13).w(FUNC(cmaster_state::background_col_w));
	map(0x20, 0x20).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void cmaster_state::amcoe2_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x04, 0x07).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0x08, 0x0b).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* DIP switches */
	map(0x10, 0x10).w(FUNC(cmaster_state::outport0_w));
	map(0x11, 0x11).w(FUNC(cmaster_state::cm_coincount_w));
	map(0x12, 0x12).w(FUNC(cmaster_state::p1_lamps_w));
	map(0x13, 0x13).w(FUNC(cmaster_state::background_col_w));
}


void goldstar_state::lucky8_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));  /* no sound... only use both ports for DSWs */
	map(0xb850, 0xb850).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xb860, 0xb860).w(FUNC(goldstar_state::p2_lamps_w));
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));    /* sound */
	map(0xc000, 0xf7ff).rom();  // could be used by some sets like super972.
	map(0xf800, 0xffff).ram();
}

void goldstar_state::flaming7_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");

	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
//  AM_RANGE(0x9a00, 0x9fff) AM_RAM
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
//  AM_RANGE(0xa200, 0xa7ff) AM_RAM
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
//  AM_RANGE(0xaa00, 0xafff) AM_RAM

//  AM_RANGE(0xb000, 0xb03f) AM_RAM
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
//  AM_RANGE(0xb0c0, 0xb0ff) AM_RAM
	map(0xb100, 0xb17f).ram().share("reel3_scroll");
	map(0xb180, 0xb7ff).ram();

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports (92) */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports (9B) */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports (90) */
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));  /* no sound... only use both ports for DSWs */
	map(0xb850, 0xb850).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xb860, 0xb860).w(FUNC(goldstar_state::p2_lamps_w));
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));    /* sound */
//  AM_RANGE(0xc000, 0xd3ff) AM_RAM
	map(0xf800, 0xffff).ram();
}
/*
  W 9A00-B7FF FF
 RW B000-B7FF 00

 Flaming7 custom hardware writes the bonus
 graphics at 9A80-9A89, and the "reel scroll"
 at B0C0-B0FF...
*/


/* need to check item by item...

    PPIs are OK:
    B800 = 9B (A, B, C, all input)
    B810 = 9B (A, B, C, all input)
    B820 = 90 (A input, B & C output)

    RAM/ROM/NVRAM are OK.

    Reels RAM and scroll are OK.
    (only reel 2 seems to be used by the game)

    Video & atr RAM are OK.

    PSG is OK.

    AY8910 ports are OK.
*/
void goldstar_state::mbstar_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));  /* no sound... only use both ports for DSWs */
	map(0xb850, 0xb850).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xb860, 0xb860).w(FUNC(goldstar_state::p2_lamps_w));
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));    /* sound */
	map(0xc000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}


WRITE8_MEMBER(wingco_state::magodds_outb850_w)
{
	// guess, could be wrong, this might just be lights


	if (data&0x20)
		m_tile_bank = 1;
	else
		m_tile_bank = 0;

	//popmessage("magodds_outb850_w %02x\n", data);

	m_fg_tilemap->mark_all_dirty();

}

WRITE8_MEMBER(wingco_state::magodds_outb860_w)
{
//  popmessage("magodds_outb860_w %02x\n", data);
}

WRITE8_MEMBER(wingco_state::fl7w4_outc802_w)
{
	m_fl7w4_id->write((data >> 6) & 0x01);
}

void wingco_state::magodds_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	// where does the extra rom data map?? it seems like it should come straight after the existing rom, but it can't if this is a plain z80?
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(wingco_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(wingco_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(wingco_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(wingco_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa900, 0xaaff).ram().w(FUNC(wingco_state::goldstar_reel3_ram_w)).share("reel3_ram"); // +0x100 compared to lucky8
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));             /* no sound... only use both ports for DSWs */
	map(0xb850, 0xb850).w(FUNC(wingco_state::magodds_outb850_w));                                /* lamps */
	map(0xb860, 0xb860).w(FUNC(wingco_state::magodds_outb860_w));                                /* watchdog */
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));                /* sound */
	map(0xc000, 0xffff).rom().region("maincpu", 0xc000);
}

void goldstar_state::kkotnoli_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram(); /* definitely no NVRAM */
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Port */
	map(0xb830, 0xb830).nopw();                                                /* no ay8910 */
	map(0xb840, 0xb840).nopw();                                                /* no ay8910 */
	map(0xb850, 0xb850).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));                /* sound */
	map(0xf800, 0xffff).ram();
}


//WRITE8_MEMBER(goldstar_state::ladylinr_outport_w)
//{
/* LAMPS (b840)...

   .... ...x
   .... ..x.
   .... .x..
   .... x...  BET
   ...x ....  SMALL/INFO
   ..x. ....  START
   .x.. ....
   x... ....
*/
//  popmessage("Output: %02X", data);
//}

void goldstar_state::ladylinr_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* DSW bank */
	map(0xb830, 0xb830).w("aysnd", FUNC(ay8910_device::address_w));             /* no sound... unused? */
	map(0xb840, 0xb840).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb850, 0xb850).nopw();                                                /* just turn off the lamps, if exist */
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));                /* sound */
	map(0xf800, 0xffff).ram();
}

void goldstar_state::wcat3_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(goldstar_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0x9000, 0x97ff).ram().w(FUNC(goldstar_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0x9800, 0x99ff).ram().w(FUNC(goldstar_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(goldstar_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xa800, 0xa9ff).ram().w(FUNC(goldstar_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xb040, 0xb07f).ram().share("reel1_scroll");
	map(0xb080, 0xb0bf).ram().share("reel2_scroll");
	map(0xb100, 0xb17f).ram().share("reel3_scroll");

	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input/Output Ports */
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));             /* no sound... only use both ports for DSWs */
	map(0xb850, 0xb850).w(FUNC(goldstar_state::p1_lamps_w));
	map(0xb870, 0xb870).w("snsnd", FUNC(sn76489_device::write));                /* sound */
//  AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_3", i8255_device, read, write)    /* Other PPI initialized? */
	map(0xd000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}


/* newer / more capable hw */
void unkch_state::unkch_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc1ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc800, 0xc9ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");

	map(0xd000, 0xd7ff).ram().share("nvram");

	map(0xd840, 0xd87f).ram().share("reel1_scroll");
	map(0xd880, 0xd8bf).ram().share("reel2_scroll");
	map(0xd900, 0xd93f).ram().share("reel3_scroll");
	map(0xdfc0, 0xdfff).ram();

	map(0xe000, 0xe7ff).ram().w(FUNC(unkch_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(unkch_state::goldstar_fg_atrram_w)).share("fg_atrram");

	map(0xf000, 0xf1ff).ram().w(FUNC(unkch_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xf200, 0xf3ff).ram().w(FUNC(unkch_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xf400, 0xf5ff).ram().w(FUNC(unkch_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf600, 0xf7ff).ram();
	map(0xf800, 0xf9ff).ram().w(FUNC(unkch_state::reel1_attrram_w)).share("reel1_attrram");
	map(0xfa00, 0xfbff).ram().w(FUNC(unkch_state::reel2_attrram_w)).share("reel2_attrram");
	map(0xfc00, 0xfdff).ram().w(FUNC(unkch_state::reel3_attrram_w)).share("reel3_attrram");
	map(0xfe00, 0xffff).ram();
}


WRITE8_MEMBER(unkch_state::coincount_w)
{
/*
  7654 3210
  ---- --x-  Payout counter (rate set with DIP switches)
  ---- -x--  Credit counter (1 pulse/10 credits)
  ---- x---  Key In counter
  --xx ----  used for something during ticket dispensing
  x--- ----  Ticket Dispenser Motor
  -x-- ---x  unused/unknown

*/

	m_ticket_dispenser->motor_w(BIT(data, 7));

	machine().bookkeeping().coin_counter_w(0, data & 0x04);  /* Credit counter */
	machine().bookkeeping().coin_counter_w(1, data & 0x08);  /* Key In counter */
	machine().bookkeeping().coin_counter_w(2, data & 0x02);  /* payout counter */

	//popmessage("coin counters: %02x", data);
}

WRITE8_MEMBER(unkch_state::unkcm_0x02_w)
{
/*  bits
  7654 3210
  ---- ---x     button lamp: Bet-A / Stop 2
  ---- --x-     button lamp: Start / Stop All
  ---- -x--     button lamp: Info / Small / Stop 3
  ---- x---     button lamp: Big
  ---x ----     button lamp: Bet-B / D-Up
  --x- ----     button lamp: Take / Stop 1
  -x-- ----     unknown/unused
  x--- ----     vblank IRQ enable

  these sets use crude PWM to dim lamp 2 which requires filament physics simulation to work properly
*/

	//popmessage("unkcm_0x02_w %02x", data);

	m_vblank_irq_enable = data & 0x80;
	if (!m_vblank_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_lamps[0] = BIT(data, 0);  /* Bet-A / Stop 2 */
	m_lamps[1] = BIT(data, 1);  /* Start / Stop All */
	m_lamps[2] = BIT(data, 2);  /* Info / Small / Stop 3 */
	m_lamps[3] = BIT(data, 3);  /* Big */
	m_lamps[4] = BIT(data, 4);  /* Bet-B / D-Up */
	m_lamps[5] = BIT(data, 5);  /* Take / Stop 1 */
}

WRITE8_MEMBER(unkch_state::unkcm_0x03_w)
{
	//popmessage("unkcm_0x03_w %02x", data);

	m_vidreg = data;

	// -x-- ----   seems to toggle when a 'normal' tilemap should be displayed instead of the reels?
}


void unkch_state::unkch_portmap(address_map &map)
{
	map.global_mask(0xff);

	map(0x01, 0x01).w(FUNC(unkch_state::coincount_w));
	map(0x02, 0x02).w(FUNC(unkch_state::unkcm_0x02_w));
	map(0x03, 0x03).w(FUNC(unkch_state::unkcm_0x03_w));

	map(0x08, 0x08).portr("IN0");
	map(0x09, 0x09).portr("IN1");
	map(0x0a, 0x0a).portr("DSW4");
	map(0x0b, 0x0b).portr("DSW3");

	map(0x10, 0x10).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x11, 0x11).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x12, 0x12).w("aysnd", FUNC(ay8910_device::address_w));
}


void unkch_state::megaline_map(address_map &map)
{
/* Reels stuff are there just as placeholder, and obviously in wrong offset */
	map(0x0000, 0x9fff).rom();

	map(0xd000, 0xd7ff).ram(); //AM_SHARE("nvram")

	map(0xd840, 0xd87f).ram().share("reel1_scroll");
	map(0xd880, 0xd8bf).ram().share("reel2_scroll");
	map(0xd900, 0xd93f).ram().share("reel3_scroll");
	map(0xdfc0, 0xdfff).ram();

	map(0xe000, 0xe7ff).ram().w(FUNC(unkch_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(unkch_state::goldstar_fg_atrram_w)).share("fg_atrram");

	map(0xf000, 0xf1ff).ram().w(FUNC(unkch_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xf200, 0xf3ff).ram().w(FUNC(unkch_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xf400, 0xf5ff).ram().w(FUNC(unkch_state::goldstar_reel3_ram_w)).share("reel3_ram");
	map(0xf600, 0xf7ff).ram();
	map(0xf800, 0xf9ff).ram().w(FUNC(unkch_state::reel1_attrram_w)).share("reel1_attrram");
	map(0xfa00, 0xfbff).ram().w(FUNC(unkch_state::reel2_attrram_w)).share("reel2_attrram");
	map(0xfc00, 0xfdff).ram().w(FUNC(unkch_state::reel3_attrram_w)).share("reel3_attrram");
	map(0xfe00, 0xffff).ram();
}

/* unknown I/O byte R/W

  PSGs:    A0 - C0 - E0
  AY8910?: 60 - 80

*/
void goldstar_state::megaline_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0xa0, 0xa0).w("sn1", FUNC(sn76489_device::write));                      /* SN76489 #1 */
	map(0xc0, 0xc0).w("sn2", FUNC(sn76489_device::write));                      /* SN76489 #2 */
	map(0xe0, 0xe0).w("sn3", FUNC(sn76489_device::write));                      /* SN76489 #3 */
	map(0x60, 0x60).w("aysnd", FUNC(ay8910_device::address_w));                 /* AY8910 control? */
	map(0x80, 0x80).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));        /* AY8910 Input? */
//  AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
//  AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
}


void unkch_state::bonusch_map(address_map &map)
{
/* Reels stuff and RAM are there just as placeholder, and obviously in wrong offset */

	map(0x0000, 0xbfff).rom();     // ok

	map(0xd800, 0xdfff).ram(); //AM_SHARE("nvram")

	map(0xe000, 0xe7ff).ram().w(FUNC(unkch_state::goldstar_fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(unkch_state::goldstar_fg_atrram_w)).share("fg_atrram");

/* just placeholders */
	map(0xf000, 0xf1ff).ram().w(FUNC(unkch_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xf200, 0xf3ff).ram().w(FUNC(unkch_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xf400, 0xf5ff).ram().w(FUNC(unkch_state::goldstar_reel3_ram_w)).share("reel3_ram");

	map(0xf640, 0xf67f).ram().share("reel1_scroll");
	map(0xf680, 0xf6bf).ram().share("reel2_scroll");
	map(0xf700, 0xf73f).ram().share("reel3_scroll");

	map(0xf800, 0xf9ff).ram().w(FUNC(unkch_state::reel1_attrram_w)).share("reel1_attrram");
	map(0xfa00, 0xfbff).ram().w(FUNC(unkch_state::reel2_attrram_w)).share("reel2_attrram");
	map(0xfc00, 0xfdff).ram().w(FUNC(unkch_state::reel3_attrram_w)).share("reel3_attrram");
}

/* Bonus Chance W-8

  clean 20h, 30h & 40h

  50h = SN76489 #1
  51h = SN76489 #2
  52h = SN76489 #3
  53h = SN76489 #4

  10h = RW
  20h = RW
  30h =  W
  40h =  W

  60h = R

*/
void goldstar_state::bonusch_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).portr("IN0");
	map(0x20, 0x20).portr("IN1");
	map(0x50, 0x50).w("sn1", FUNC(sn76489_device::write));      /* SN76489 #1 */
	map(0x51, 0x51).w("sn2", FUNC(sn76489_device::write));      /* SN76489 #2 */
	map(0x52, 0x52).w("sn3", FUNC(sn76489_device::write));      /* SN76489 #3 */
	map(0x53, 0x53).w("sn4", FUNC(sn76489_device::write));      /* SN76489 #4 */
	map(0x60, 0x60).portr("IN3");
}

void unkch_state::feverch_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();

	map(0xc000, 0xc7ff).ram().w(FUNC(unkch_state::goldstar_fg_atrram_w)).share("fg_atrram");
	map(0xc800, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(unkch_state::goldstar_fg_vidram_w)).share("fg_vidram");

	// placeholders to appease validation, should be 0x200 each.
	map(0xe000, 0xe000).ram().w(FUNC(unkch_state::goldstar_reel1_ram_w)).share("reel1_ram");
	map(0xe200, 0xe200).ram().w(FUNC(unkch_state::goldstar_reel2_ram_w)).share("reel2_ram");
	map(0xe400, 0xe400).ram().w(FUNC(unkch_state::goldstar_reel3_ram_w)).share("reel3_ram");

	// placeholders to appease validation, should be 0x40 each.
	map(0xe640, 0xe640).ram().share("reel1_scroll");
	map(0xe680, 0xe680).ram().share("reel2_scroll");
	map(0xe700, 0xe700).ram().share("reel3_scroll");

	// placeholders to appease validation, should be 0x200 each.
	map(0xe800, 0xe800).ram().w(FUNC(unkch_state::reel1_attrram_w)).share("reel1_attrram");
	map(0xea00, 0xea00).ram().w(FUNC(unkch_state::reel2_attrram_w)).share("reel2_attrram");
	map(0xec00, 0xec00).ram().w(FUNC(unkch_state::reel3_attrram_w)).share("reel3_attrram");

	map(0xf800, 0xffff).ram();
}

void goldstar_state::feverch_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).w("sn1", FUNC(sn76489_device::write));
	map(0x28, 0x28).w("sn2", FUNC(sn76489_device::write));
	map(0x30, 0x30).w("sn3", FUNC(sn76489_device::write));
	//map(0x38, 0x3b)
}

static INPUT_PORTS_START( cmv4_player )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_coins )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B") /* Service coin in some cases */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME("Coin D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_service )
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")  //  on some sets a DSW must be on/off to access this menu
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_dsw1 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Hold Pair" )                 PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" )         PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, "Payout Switch" )
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, "Lose" )
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Game" )            PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_dsw2 )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Lowest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Highest)" )
	PORT_DIPNAME( 0x18, 0x18, "Hopper Limit" )              PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x02, "100+ Odds Sound" )           PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key In Type" )               PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_dsw3 )
	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4_dsw4 )
	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( cmv4_dsw5 )
	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )                  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cb3_dsw1 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )                PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Tickets" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )         PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x08, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Lose" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "Double Up Game" )            PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb3_dsw3 )
	PORT_INCLUDE( cmv4_dsw3 )
	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" )   PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )        PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )    PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" )   PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Ticket/5 Credits" )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Ticket/10 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Ticket/25 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Ticket/50 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( cb3_dsw4 )
	PORT_INCLUDE( cmv4_dsw4 )
	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x10, 0x10, "Coin D Type" )   PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "C-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "D-Type (Tickets)" )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )   PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb3_dsw5 )
	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )   PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )     PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )     PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x20, "Double Up Girl" )    PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin In Rate" )      PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END


static INPUT_PORTS_START( cmv801 )
	PORT_INCLUDE( cmv4_player )

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	/* Hold Pair OK - use Take button */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	/* Double Up Game OK */
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* Display Of Payout Limit not working; all others OK */

	PORT_INCLUDE( cmv4_dsw5 )
	PORT_MODIFY("DSW5")
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	PORT_DIPNAME( 0x40, 0x40, "Card Shuffle Animation" )    PORT_DIPLOCATION("DSW5:7")  /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Test Mode For Disp. Of Doll not working */
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4 )
	PORT_INCLUDE( cmv4_player )

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	/* Hold Pair OK - use Take button */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x10, "60%" )
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "38%" )
	PORT_DIPSETTING(    0x05, "46%" )
	PORT_DIPSETTING(    0x04, "54%" )
	PORT_DIPSETTING(    0x03, "62%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "78%" )
	PORT_DIPSETTING(    0x00, "86%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not working */
INPUT_PORTS_END

static INPUT_PORTS_START( cmaster )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not working */
INPUT_PORTS_END

static INPUT_PORTS_START( cmasterb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Stop All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not checked */
INPUT_PORTS_END


static INPUT_PORTS_START( cmezspin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	/* Hold Pair OK */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")      /* OK */
	PORT_DIPSETTING(    0x00, "Win" )
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x10, "80%" )
	/* Double Up Game OK */
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2")    /* OK */
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Hopper Limit not checked */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) - not checked */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )
	PORT_MODIFY("DSW4")
	/* Credit Limit not checked */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Type Of Coin D OK */
	/* Min. Bet For Bonus Play OK */
	/* Reel Speed OK */
	/* Hopper Out By Coin A OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition for 3 Kind Of Bonus OK */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not checked */
INPUT_PORTS_END


static INPUT_PORTS_START( cmasterc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Big / Stop 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("D-UP / Stop 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Small / Info / Stop 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not checked */
INPUT_PORTS_END


static INPUT_PORTS_START( cmasterg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Stop All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_INCLUDE( cmv4_dsw1 )
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not checked */
INPUT_PORTS_END


static INPUT_PORTS_START( cmast91 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Bet" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Stop All")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")  /* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )         PORT_DIPLOCATION("DSW1:2")  /* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")  /* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")  /* OK */
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPSETTING(    0x00, "Loss" )
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Game" )            PORT_DIPLOCATION("DSW1:6")  /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	/* Hopper Limit OK */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw5 )
	PORT_MODIFY("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:1")  /* normally Display of Doll On Demo, but no whores in this set */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	PORT_DIPNAME( 0x20, 0x20, "Show Odds In Double Up Game" )   PORT_DIPLOCATION("DSW5:6")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Skill Stop" )                    PORT_DIPLOCATION("DSW5:7")  /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:8")  /* normally Test Mode For Disp. Of Doll, but no whores in this set */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cmast99 )
	PORT_START("IN0") // called "PLAYER" in input test
	PORT_DIPNAME( 0x01, 0x01, "0-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "0-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "0-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "0-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "0-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "0-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "0-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "0-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1") // called "COIN" in input test
	PORT_DIPNAME( 0x01, 0x01, "1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("IN2") // called "TEST" in input test
	PORT_DIPNAME( 0x01, 0x01, "2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Min Bet" )              PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "15" )
	PORT_DIPSETTING(    0x02, "30" )
	PORT_DIPSETTING(    0x01, "45" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x04, "Double Mode" )               PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, "No (collect)" )
	PORT_DIPSETTING(    0x04, "Yes (double)" )
	PORT_DIPNAME( 0x18, 0x18, "Max Bet" )                   PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "15" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x10, "60" )
	PORT_DIPSETTING(    0x18, "99" )
	PORT_DIPNAME( 0x20, 0x20, "Double Picture" )            PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "Fruit" )
	PORT_DIPSETTING(    0x00, "???" ) // not listed in manual
	PORT_DIPNAME( 0x40, 0x40, "Payout Rate" )               PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "100" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:8") // not listed in manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x06, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x08, 0x08, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x08, "70%" )
	PORT_DIPNAME( 0x30, 0x20, "Coin In Rate" )              PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Key In Rate" )               PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/110 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/120 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )

	PORT_START("DSW3") // manual doesn't have explicit defaults, so taking first listed setting for each switch as default
	PORT_DIPNAME( 0x01, 0x00, "Bonus 2 Poll Mode" )                                         PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus 3 Poll Mode" )                                         PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "???" )                                                       PORT_DIPLOCATION("DSW3:3") // no name given in manual, but settings listed
	PORT_DIPSETTING(    0x04, "30-40-50" )
	PORT_DIPSETTING(    0x00, "40-55-70" )
	PORT_DIPNAME( 0x08, 0x08, "When Watermelon Appear in Center Credit of Bonus Up Mode" )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, "1-3-5" )
	PORT_DIPSETTING(    0x00, "2-4-6" )
	PORT_DIPNAME( 0x10, 0x10, "When Bet, Credit of Bonus Up Mode" )                         PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, "1-3-5" )
	PORT_DIPSETTING(    0x00, "2-4-6" )
	PORT_DIPNAME( 0x20, 0x20, "The Max Credit of Bonus 1-3" )                               PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, "999-1999-3999" )
	PORT_DIPSETTING(    0x00, "2000-3000-4500" )
	PORT_DIPNAME( 0x40, 0x40, "Start Up Credit of Bonus 1-3" )                              PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x00, "330-660-990" )
	PORT_DIPSETTING(    0x40, "499-999-1999" )
	PORT_DIPNAME( 0x80, 0x80, "Start Up Credit of Extra Bonus" )                            PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, "330" )
	PORT_DIPSETTING(    0x00, "550" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x00, "Max Pay Credit Limit" )  PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07,   "5,000" )
	PORT_DIPSETTING(    0x06,  "10,000" )
	PORT_DIPSETTING(    0x05,  "15,000" )
	PORT_DIPSETTING(    0x04,  "20,000" )
	PORT_DIPSETTING(    0x03,  "30,000" )
	PORT_DIPSETTING(    0x02,  "40,000" )
	PORT_DIPSETTING(    0x01,  "50,000" )
	PORT_DIPSETTING(    0x00, "100,000" )
	PORT_DIPNAME( 0x08, 0x08, "Display Credit Limit" )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Add On Bonus Poll" )     PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x00, "6-3-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPNAME( 0x20, 0x20, "Add On Bonus Min Bet" )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x40, 0x40, "Game Speed" )            PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, "Alarm Sound" )           PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x03, "Key In Limit" )                          PORT_DIPLOCATION("DSW5:1,2")
	PORT_DIPSETTING(    0x00,  "1,000" )
	PORT_DIPSETTING(    0x01,  "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x03, "20,000" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Girl" )                            PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Credit Earned, Profit Table Set Zero" )  PORT_DIPLOCATION("DSW5:4,5,6")
	PORT_DIPSETTING(    0x00,  "20,000" )
	PORT_DIPSETTING(    0x08,  "30,000" )
	PORT_DIPSETTING(    0x10,  "40,000" )
	PORT_DIPSETTING(    0x18,  "50,000" )
	PORT_DIPSETTING(    0x20,  "70,000" )
	PORT_DIPSETTING(    0x28, "100,000" )
	PORT_DIPSETTING(    0x30, "200,000" )
	PORT_DIPSETTING(    0x38, "990,000" )
	PORT_DIPNAME( 0x40, 0x00, "Bonus 2 Poll Rate" )                     PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Bonus 3 Poll Rate" )                     PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END


static INPUT_PORTS_START( goldstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // appear in the input test but seems to lack function
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    // appear in the input test but seems to lack function
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red / 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / 1 / Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue / D-UP / 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")   /* hopper empty */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_INCLUDE( cb3_dsw1 )
	PORT_MODIFY("DSW1")
	/* Game Style not checked */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	/* Double Up Game OK */
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )                   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3" ) /* Does this work?  Settings screen always shows "28F3%". */
	PORT_DIPSETTING(    0x07, "40%" )
	PORT_DIPSETTING(    0x06, "45%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x04, "55%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )           PORT_DIPLOCATION("DSW2:7" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" ) PORT_DIPLOCATION("DSW2:8" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3") /* Neither of these work.  Does the manual say this is what they do, or is it just nonsense? */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW3:1,2" )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x40, "Coin C" )                PORT_DIPLOCATION("DSW3:3,4" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_INCLUDE( cmv4_dsw4 )
	PORT_MODIFY("DSW4")
	/* Credit Limit OK */
	/* Display Of Payout Limit OK */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")  /* not checked */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Min. Bet For Bonus Play OK */
	/* Reel Speed OK */
	PORT_DIPNAME( 0x80, 0x00, "Ticket Payment" )    PORT_DIPLOCATION("DSW4:8")  /* not checked */
	PORT_DIPSETTING(    0x80, "1 Ticket/100" )
	PORT_DIPSETTING(    0x00, "Pay All" )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW7")  /* ??? Where am I hooked to??? */
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x00, "Show Woman" )            PORT_DIPLOCATION("DSW7:1" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chrygld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Bonus Game")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / Card 4")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE( cb3_dsw1 )
	PORT_MODIFY("DSW1")
	/* Game Style not checked */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	/* Double Up Game Pay Rate OK */
	/* Double Up Game OK */
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	/* Hopper limit not checked */
	/* 100+ Odds Sound not checked */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:7")      /* normally Key In Type but doesn't affect rate for this game */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")      /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")      /* OK - unused value also produces 1C/10C */
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" )   PORT_DIPLOCATION("DSW3:5,6")    /* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Ticket/5 Credits" )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Ticket/10 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Ticket/20 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Ticket/50 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_INCLUDE( cb3_dsw4 )    /* all OK */

	PORT_INCLUDE( cb3_dsw5 )    /* DSW5 is not connected yet. Where the hell is connected? */
INPUT_PORTS_END

// dip switches from manual, values & inputs are a guess from cmasterb
static INPUT_PORTS_START( chryangl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Stop All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )       PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )          PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )     PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )         PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "40%" )
	PORT_DIPSETTING(    0x06, "45%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x04, "55%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )           PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")  // note in manual says "Reverse these settings" for entire DSW3 ???
	PORT_DIPNAME( 0x01, 0x01, "Unused" )                PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )         PORT_DIPLOCATION("DSW3:2,3")
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x00, "Credit Back" )           PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(    0x18, "10 In 1 Back" )
	PORT_DIPSETTING(    0x10, "8 In 1 Back" )
	PORT_DIPSETTING(    0x08, "6 In 1 Back" )
	PORT_DIPSETTING(    0x00, "No Coins Back" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Unused" )                PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x05, "Credit Limit" )              PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" )   PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )            PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" )   PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )                PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x00, "Key In Rate" )   PORT_DIPLOCATION("DSW5:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" )   PORT_DIPLOCATION("DSW5:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" )   PORT_DIPLOCATION("DSW5:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" )   PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )
INPUT_PORTS_END

/* no manual - best guesses */
static INPUT_PORTS_START( tonypok )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Big / Red")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / D-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Small / Black")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Deal / Draw")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")  /* needs to be mapped correctly */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")  /* unknown */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")  /* unknown */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")  /* unknown */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")  /* unknown */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")  /* unknown */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")  /* unknown */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")  /* unknown */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")  /* unknown */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x01, "45%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x04, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPNAME( 0x18, 0x18, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")    /* not checked */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:6")      /* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key In Type" )           PORT_DIPLOCATION("DSW2:7")      /* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x80, "Coin D Rate" )           PORT_DIPLOCATION("DSW2:8")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" )       PORT_DIPLOCATION("DSW3:1,2")        /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )    PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )        PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x04, "Coin A Rate" )       PORT_DIPLOCATION("DSW3:3")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")          /* unknown */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")          /* unknown */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")          /* unknown */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")          /* unknown */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")          /* unknown */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Limit" )         PORT_DIPLOCATION("DSW4:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "1,000" )
	PORT_DIPSETTING(    0x05, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x08, 0x08, "Instant W-Up Game" )     PORT_DIPLOCATION("DSW4:4")      /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   /* play W-up game right away from bet screen! */
	PORT_DIPNAME( 0x10, 0x10, "Attract Mode Demo" )     PORT_DIPLOCATION("DSW4:5")      /* OK */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )  /* Title screen only */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   /* Title screen and demo */
	PORT_DIPNAME( 0x20, 0x20, "Bonus Game Type" )       PORT_DIPLOCATION("DSW4:6")      /* OK */
	PORT_DIPSETTING(    0x20, "Big / Small" )
	PORT_DIPSETTING(    0x00, "Black / Red" )
	PORT_DIPNAME( 0x40, 0x40, "Card Type" )             PORT_DIPLOCATION("DSW4:7")      /* OK */
	PORT_DIPSETTING(    0x40, "Standard" )
	PORT_DIPSETTING(    0x00, "Jets" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:8")      /* OK */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Lowest Winning Hand" )   PORT_DIPLOCATION("DSW5:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "2 Pair" )
	PORT_DIPSETTING(    0x06, "Any Pair" )
	PORT_DIPSETTING(    0x05, "6s Or Higher" )
	PORT_DIPSETTING(    0x04, "10s Or Higher" )
	PORT_DIPSETTING(    0x03, "Jacks Or Higher" )
	PORT_DIPSETTING(    0x02, "Queens Or Higher" )
	PORT_DIPSETTING(    0x01, "Kings Or Higher" )
	PORT_DIPSETTING(    0x00, "Ace Pair" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW5:4")      /* unknown */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Number Of Draws" )       PORT_DIPLOCATION("DSW5:5")      /* OK */
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, "Joker In Deck" )         PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )           PORT_DIPLOCATION("DSW5:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20")
	PORT_DIPSETTING(    0x40, "30" )
INPUT_PORTS_END

/* taken from manual - (it's a starting point)
   consider everything unverified
   not all DIP banks are actually hooked up as I/O map is currently based on what crazybon actually reads
*/
static INPUT_PORTS_START( pkrmast )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red / 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / 1 / Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue / D-UP / 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1:1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1:2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1:3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1:4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1:5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1:6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1:7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1:8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2:1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2:2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2:3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2:4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2:5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2:6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2:7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2:8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Freeze Pair On Line" )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out" )            PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Type Of Payout" )        PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-Up '7'" )              PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )         PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x20, 0x20, "W-Up Game" )             PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )               PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0x40, "64" )
	PORT_DIPSETTING(    0x00, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key In Type" )           PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Type Of Coin D" )        PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, "C Type" )
	PORT_DIPSETTING(    0x80, "D Type" )

	PORT_START("DSW3-0")
	PORT_DIPNAME( 0x03, 0x00, "Key In Rate" )           PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* A Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )    PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C) )         PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* B Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x00, "Coin A Rate" )           PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3-1")
	PORT_DIPNAME( 0x03, 0x00, "Coin D Rate" )           PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x00) /* C Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )    PORT_CONDITION("DSW2",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C) )         PORT_CONDITION("DSW2",0x80,EQUALS,0x80) /* D Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x80,EQUALS,0x80)
	PORT_DIPNAME( 0x0c, 0x00, "Coin C Rate" )           PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x00, "Credit Limit" )                  PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5000" )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x05, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Credit Limit" )          PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bet Limit For Center Super 7" )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "Limited" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min Bet" )            PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )                    PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper Out By Coin A" )          PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "On" )
	PORT_DIPSETTING(    0x00, "Off" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                 PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Bonus" )    PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x18, "3-2-1" )
	PORT_DIPSETTING(    0x10, "6-3-1" )
	PORT_DIPSETTING(    0x08, "9-5-1" )
	PORT_DIPSETTING(    0x00, "12-7-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Doll On Fruit & Cherry Bonus" )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Coin Out Rate" )                 PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x00, "Run Cards In W-Up" )             PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW6")
	PORT_DIPNAME( 0x01, 0x00, "Card Type" )                     PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(    0x01, "Cards" )
	PORT_DIPSETTING(    0x00, "Missiles" )
	PORT_DIPNAME( 0x02, 0x02, "Type Of W-Up Game" )             PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(    0x00, "Big / Small" )
	PORT_DIPSETTING(    0x02, "Red / Black" )
	PORT_DIPNAME( 0x04, 0x00, "Hold After 1st Hold" )           PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(    0x04, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x08, 0x00, "Royal Win" )                     PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(    0x08, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0x10, 0x00, "Joker" )                         PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(    0x10, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPNAME( 0xe0, 0x00, "One Pair Win Type" )             PORT_DIPLOCATION("DSW6:6,7,8")
	PORT_DIPSETTING(    0x80, "No Win" )
	PORT_DIPSETTING(    0x60, "Any Pair" )
	PORT_DIPSETTING(    0x40, "6s & Better" )
	PORT_DIPSETTING(    0xe0, "10s & Beter" )
	PORT_DIPSETTING(    0xc0, "Jacks & Better" )
	PORT_DIPSETTING(    0xa0, "Queens & Better" )
	PORT_DIPSETTING(    0x20, "Kings & Better" )
	PORT_DIPSETTING(    0x00, "Ace Pair" )

	PORT_START("DSW7")
	PORT_DIPNAME( 0x03, 0x01, "Minimum Bet To Play" )           PORT_DIPLOCATION("DSW7:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x04, "Keys Panel Type" )               PORT_DIPLOCATION("DSW7:3,4")
	PORT_DIPSETTING(    0x04, "A Type" )
	PORT_DIPSETTING(    0x0c, "B Type" )
	PORT_DIPSETTING(    0x08, "C Type" )
	PORT_DIPSETTING(    0x00, "D Type" )
	PORT_DIPNAME( 0x10, 0x00, "Chance Bonus" )                  PORT_DIPLOCATION("DSW7:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound At Run & Open Cards" )     PORT_DIPLOCATION("DSW7:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0x80, "Main Game Type" )                PORT_DIPLOCATION("DSW7:7,8")
	PORT_DIPSETTING(    0x40, "Cherry Master Only - Full Demo" )
	PORT_DIPSETTING(    0xc0, "Poker Only - Full Demo" )
	PORT_DIPSETTING(    0x80, "Full Demo Of Both Games" )
	PORT_DIPSETTING(    0x00, "Logo Only Demo Of Both Games" )
INPUT_PORTS_END

static INPUT_PORTS_START( chry10 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Bonus Game")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / Card 4")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE( cb3_dsw1 )
	PORT_MODIFY("DSW1")
	/* Game Style not checked */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4")  /* normally '7' In Double Up Game but doesn't seem to do anything */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Double Up Game Pay Rate OK */
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x01, "45%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x07, "80%" )
	/* Hopper Limit not checked */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK - note that definition of A-Type and B-Type are reversed compared to cmv4 */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )        PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )    PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")  /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")  /* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )        PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, "2 Coins/10 Credits" )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )     PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Coin/20 Credits" )     PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Ticket/50 Credits" )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Ticket/100 Credits" )  PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Ticket/200 Credits" )  PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Ticket/500 Credits" )  PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")  /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/100 Credits" )

	PORT_INCLUDE( cb3_dsw4 )    /* all OK */

	PORT_INCLUDE( cb3_dsw5 )
	/* DSW5-1 not checked */
	/* Coin In Limit not working */
	/* Coin Out Rate not checked */
	/* Double Up Girl not working (always shows face in demo) */
	/* Coin In Rate not checked */
INPUT_PORTS_END

static INPUT_PORTS_START( ncb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Start / Stop All / Card 4")

	PORT_INCLUDE( cmv4_coins )

	PORT_INCLUDE( cmv4_service )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Start / Stop All / Card 4")

	PORT_INCLUDE( cb3_dsw1 )
	/* Game Style not checked */
	/* Hopper Out Switch not checked */
	/* Payout Mode not checked */
	/* '7' In Double Up Game OK */
	/* Double Up Game Pay Rate OK */
	/* Double Up Game OK */
	/* Bet Max OK */

	PORT_INCLUDE( cmv4_dsw2 )
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	/* Hopper Limit not checked */
	/* 100+ Odds Sound not checked */
	/* Key In Type OK - note that definition of A-Type and B-Type are reversed compared to cmv4 */
	/* Center Super 7 Bet Limit related with Min. Bet For Bonus Play (DSW4-6) */

	PORT_INCLUDE( cb3_dsw3 )    /* all OK */

	PORT_INCLUDE( cb3_dsw4 )    /* all OK */

	PORT_INCLUDE( cb3_dsw5 )
	/* DSW5-1 not checked */
	/* Coin In Limit not working */
	/* Coin Out Rate not checked */
	/* Double Up Girl not working (always shows face in demo) */
	/* Coin In Rate not checked */
INPUT_PORTS_END

static INPUT_PORTS_START( cb3a )
	PORT_INCLUDE( ncb3 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )     PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x10, "80%" )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8 )
	PORT_START("IN0")   /* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")   /* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME("Coin D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")

	PORT_START("IN4")   /* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Coin Switch" )        PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")      /* OK */
	PORT_DIPSETTING(    0x08, "Lose" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "Double Up Game" )            PORT_DIPLOCATION("DSW1:6")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Special Odds" )              PORT_DIPLOCATION("DSW1:7,8")    /* doesn't seem to actually do anything - in W-4 manual but probably unused in these game */
	PORT_DIPSETTING(    0xc0, "None" )
	PORT_DIPSETTING(    0x80, "Limited to X 300 (X 1000)" )
	PORT_DIPSETTING(    0x40, "Limited to X 500 (X 5000)" )
	PORT_DIPSETTING(    0x00, "Limited to X 1000 (X 10000)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x06, "83%" )
	PORT_DIPSETTING(    0x05, "86%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "95%" )
	PORT_DIPSETTING(    0x01, "98%" )
	PORT_DIPSETTING(    0x00, "101%" )  /* ??? */
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )              PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "Over 100 Bet Sound" )        PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Paytable Settings" )         PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "Low Pay" )
	PORT_DIPSETTING(    0x00, "High Pay" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Game Type" )       PORT_DIPLOCATION("DSW2:8")      /* OK */
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")  /* marked as DSW4 in manual */
	PORT_DIPNAME( 0x0f, 0x07, "Coin D Rate" )       PORT_DIPLOCATION("DSW4:1,2,3,4")  /* OK - all other values are 10C/1C */
	PORT_DIPSETTING(    0x0f, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x70, 0x60, "Coin C Rate" )       PORT_DIPLOCATION("DSW4:5,6,7")  /* OK - all other values are 10C/1C */
	PORT_DIPSETTING(    0x70, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "9 Coins/1 Credit" )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, "4th Coin" )          PORT_DIPLOCATION("DSW4:8")      /* OK */
	PORT_DIPSETTING(    0x80, "As Coin A" )
	PORT_DIPSETTING(    0x00, "As Hopper Line" )

	PORT_START("DSW4")  /* marked as DSW3 in manual */
	PORT_DIPNAME( 0x07, 0x07, "Key In Rate" )       PORT_DIPLOCATION("DSW3:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/60 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Coin A Rate" )       PORT_DIPLOCATION("DSW3:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )    /* manual says 1C/8C */
	PORT_DIPSETTING(    0x28, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8a )
	PORT_INCLUDE( lucky8 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8b )
	PORT_INCLUDE( lucky8 )

	PORT_MODIFY("IN1")      /* Player 2 controls not used in this set */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "75%" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "58%" )
	PORT_DIPSETTING(    0x06, "62%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x02, "78%" )
	PORT_DIPSETTING(    0x01, "82%" )
	PORT_DIPSETTING(    0x00, "86%" )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8d )
	PORT_INCLUDE( lucky8 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "43%" )
	PORT_DIPSETTING(    0x06, "47%" )
	PORT_DIPSETTING(    0x05, "53%" )
	PORT_DIPSETTING(    0x04, "57%" )
	PORT_DIPSETTING(    0x03, "63%" )
	PORT_DIPSETTING(    0x02, "67%" )
	PORT_DIPSETTING(    0x01, "73%" )
	PORT_DIPSETTING(    0x00, "79%" )
INPUT_PORTS_END

static INPUT_PORTS_START( ns8linew )
	PORT_START("IN0")   /* b800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* $b801 - P2 Controls... Leftover? Once switched all lamps turn off and no P2 big/small inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")   /* $b802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* $b810 - Money in */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B");
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME("Coin D");
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C");
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")

	PORT_START("IN4")   /* $b811 - Service controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Coin Switch" )        PORT_DIPLOCATION("DSW1:2")      /* not checked */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")      /* not checked */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x18, 0x18, "Double-Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:4,5")    /* OK */
	PORT_DIPSETTING(    0x18, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Special Odds" )              PORT_DIPLOCATION("DSW1:7,8")    /* not checked */
	PORT_DIPSETTING(    0xc0, "None" )
	PORT_DIPSETTING(    0xb0, "Limited to x300 (x1000)" )
	PORT_DIPSETTING(    0x40, "Limited to x500 (x5000)" )
	PORT_DIPSETTING(    0x00, "Limited to x1000 (x10000)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2")    /* OK */
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up Game" )        PORT_DIPLOCATION("DSW2:3")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")    /* not checked */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "Over 100 Bet Sound" )    PORT_DIPLOCATION("DSW2:6")      /* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Odds Table" )            PORT_DIPLOCATION("DSW2:7")      /* not checked */
	PORT_DIPSETTING(    0x40, "A - Low" )
	PORT_DIPSETTING(    0x00, "B - High" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:8")      /* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x07, "Coin D Rate" )           PORT_DIPLOCATION("DSW3:1,2,3,4")    /* OK - all other values are all 10C/1C */
	PORT_DIPSETTING(    0x0f, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x70, 0x60, "Coin C Rate" )       PORT_DIPLOCATION("DSW3:5,6,7")          /* OK - all other values are 10C/1C */
	PORT_DIPSETTING(    0x70, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "9 Coins/1 Credit" )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, "4th Coin" )          PORT_DIPLOCATION("DSW3:8")              /* OK */
	PORT_DIPSETTING(    0x80, "As Coin A" )
	PORT_DIPSETTING(    0x00, "As Hopper Line" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In Rate" )       PORT_DIPLOCATION("DSW4:1,2,3")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin /10 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin /20 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin /25 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin /40 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin /50 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin /60 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin /100 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Coin A Rate" )       PORT_DIPLOCATION("DSW4:4,5,6")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )    // manual says 1c/8c
	PORT_DIPSETTING(    0x28, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")              /* not checked */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")              /* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ns8linwa )
	PORT_START("IN0")   /* b800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* $b801 - P2 Controls... Not set */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* $b802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* $b810 - Money in */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B");
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME("Coin D");
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C");
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")

	PORT_START("IN4")   /* $b811 - Service controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:1")  // not checked
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Coin Switch" )        PORT_DIPLOCATION("DSW1:2")  // not checked
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )               PORT_DIPLOCATION("DSW1:3")  // not checked
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:4")  // not checked
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Double-Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5,6")  // OK
	PORT_DIPSETTING(    0x00, "60%" )  // OK
	PORT_DIPSETTING(    0x10, "65%" )  // OK
	PORT_DIPSETTING(    0x20, "70%" )  // OK
	PORT_DIPSETTING(    0x30, "75%" )  // OK
	PORT_DIPNAME( 0xc0, 0x00, "Special Odds" )              PORT_DIPLOCATION("DSW1:7,8")  // not checked
	PORT_DIPSETTING(    0xc0, "None" )
	PORT_DIPSETTING(    0xb0, "Limited to x300 (x1000)" )
	PORT_DIPSETTING(    0x40, "Limited to x500 (x5000)" )
	PORT_DIPSETTING(    0x00, "Limited to x1000 (x10000)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2")  // OK
	PORT_DIPSETTING(    0x03, "58%" )  // OK
	PORT_DIPSETTING(    0x02, "62%" )  // OK
	PORT_DIPSETTING(    0x01, "66%" )  // OK
	PORT_DIPSETTING(    0x00, "70%" )  // OK
	PORT_DIPNAME( 0x04, 0x04, "Double Up Game" )        PORT_DIPLOCATION("DSW2:3")  // not checked
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")  // not checked
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "Over 100 Bet Sound" )    PORT_DIPLOCATION("DSW2:6")  // not checked
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Odds Table" )            PORT_DIPLOCATION("DSW2:7")  // not checked
	PORT_DIPSETTING(    0x40, "A - Low" )
	PORT_DIPSETTING(    0x00, "B - High" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:8")  // not checked
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x07, "Coin D Rate" )           PORT_DIPLOCATION("DSW3:1,2,3,4")  // not checked
	PORT_DIPSETTING(    0x0f, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x70, 0x60, "Coin C Rate" )       PORT_DIPLOCATION("DSW3:5,6,7")  // not checked
	PORT_DIPSETTING(    0x70, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "9 Coins/1 Credit" )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, "4th Coin" )          PORT_DIPLOCATION("DSW3:8")  // not checked
	PORT_DIPSETTING(    0x80, "As Coin A" )
	PORT_DIPSETTING(    0x00, "As Hopper Line" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x03, "Key In Rate" )       PORT_DIPLOCATION("DSW4:1,2,3")  // OK
	PORT_DIPSETTING(    0x00, "1 Pulse / 5 Credits" )  // OK
	PORT_DIPSETTING(    0x01, "1 Pulse / 10 Credits" )  // OK
	PORT_DIPSETTING(    0x02, "1 Pulse / 20 Credits" )  // OK
	PORT_DIPSETTING(    0x03, "1 Pulse / 100 Credits" )  // OK
	PORT_DIPSETTING(    0x04, "1 Pulse / 110 Credits" )  // OK
	PORT_DIPSETTING(    0x05, "1 Pulse / 120 Credits" )  // OK
	PORT_DIPSETTING(    0x06, "1 Pulse / 130 Credits" )  // OK
	PORT_DIPSETTING(    0x07, "1 Pulse / 500 Credits" )  // OK
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")  // not checked
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Coin A Rate" )       PORT_DIPLOCATION("DSW4:5,6")  // OK
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )  // OK
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )  // OK
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )  // OK
	PORT_DIPSETTING(    0x30, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")  // not checked
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")  // not checked
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( luckylad ) // CHECK & FIX ME
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

static INPUT_PORTS_START( ladylinr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Muenze (Hopper Coin")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_J) PORT_NAME("Hopper Voll (Hopper Fill)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Abschreib (Payout)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Aufsteller (Supervisor)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Buchhaltung (Bookkeeping)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Hoch (High) / Stop 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble (D-UP)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Nehmen (Take)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Setzen (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Tief (Low) / Stop 1 / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Start / Stop 2")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Remote Credits" )    PORT_DIPLOCATION("DSW1:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "20 credits" )
	PORT_DIPSETTING(    0x10, "50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Coin B & C" )        PORT_DIPLOCATION("DSW1:6")  /* OK */
	PORT_DIPSETTING(    0x00, "10 credits" )
	PORT_DIPSETTING(    0x20, "20 credits" )
	PORT_DIPNAME( 0x40, 0x40, "Reels Speed" )       PORT_DIPLOCATION("DSW1:7")  /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Input Test Mode" )   PORT_DIPLOCATION("DSW1:8")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

/*  there are 2 extra DSW banks...
    both are tied to a daughterboard, maybe hooked to a device.
    they are not related to the original hardware, and are not
    listed in the Input Test Mode.
*/
INPUT_PORTS_END

static INPUT_PORTS_START( kkotnoli )
	PORT_START("IN0")   /* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")   /* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin1 1 coin/1 credit
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin2 10 coins/1 credit
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin3 2 coins/1 credit
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  // Coin4 1 coin/10 credits

	PORT_START("IN4")   /* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* dips 1-7 appear to do nothing */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double Up Game Type" )   PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" ) /* Fix me - reel graphics do not show in game */
	PORT_DIPSETTING(    0x00, "Flowers (Big/Small)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bingowng )
	PORT_START("IN0")   /* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")   /* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME( "Coin D" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME( "Coin C" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME( "Coin B" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME( "Coin A" )

	PORT_START("IN4")   /* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Cabinet Style" )             PORT_DIPLOCATION("DSW1:1")      /* not checked */
	PORT_DIPSETTING(    0x00, "Cocktail" )
	PORT_DIPSETTING(    0x01, "Upright" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Coin Switch")         PORT_DIPLOCATION("DSW1:2")      /* not checked */
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Type" )               PORT_DIPLOCATION("DSW1:3")      /* not checked */
	PORT_DIPSETTING(    0x04, "Take Button" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x18, 0x18, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:4,5")    /* OK */
	PORT_DIPSETTING(    0x18, "75%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x08, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
/*  On a W-4 PCB these are used as: "Special Odds-Prohibition Of Winning...(Odds B)" - see DSW2-7
    PORT_DIPNAME( 0x80, 0x00, "Special Odds" )              PORT_DIPLOCATION("DSW1:7,8")
    PORT_DIPSETTING(    0x00, "None" )
    PORT_DIPSETTING(    0x40, "x300 (x1000)" )
    PORT_DIPSETTING(    0x80, "x500 (x5000" )
    PORT_DIPSETTING(    0xc0, "x1000 (x10000)
*/

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, "Main Game Rate" )        PORT_DIPLOCATION("DSW2:1,2")    /* OK */
	PORT_DIPSETTING(    0x03, "Very Easy" )
	PORT_DIPSETTING(    0x02, "Easy" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Very Hard" )
	PORT_DIPNAME( 0x04, 0x00, "Double Up Game" )        PORT_DIPLOCATION("DSW2:3")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")    /* not checked */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "Sound for 100+ Bet" )    PORT_DIPLOCATION("DSW2:6")      /* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
/*  On a W-4 PCB these are used as:
    PORT_DIPNAME( 0x40, 0x40, "Odds" )                  PORT_DIPLOCATION("DSW2:7")
    PORT_DIPSETTING(    0x40, "Type A" )
    PORT_DIPSETTING(    0x00, "Type B" )
    PORT_DIPNAME( 0x80, 0x80, "Type Of W-Up Game" )     PORT_DIPLOCATION("DSW2:8")
    PORT_DIPSETTING(    0x80, "Slots" )
    PORT_DIPSETTING(    0x00, "Big/Small Card" )
*/

	/* On a W-4 PCB DSW3 & DSW4 are reversed and all dips on DSW4 are set to off! */
	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x07, "Coin D Rate" )           PORT_DIPLOCATION("DSW3:1,2,3,4")    /* OK - other values are all 10C/1C */
	PORT_DIPSETTING(    0x0f, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x70, 0x60, "Coin C Rate" )           PORT_DIPLOCATION("DSW3:5,6,7")      /* OK - other values are 10C/1C */
	PORT_DIPSETTING(    0x70, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, "9 Coins/1 Credit" )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bingownga )
	PORT_INCLUDE( bingowng )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Coin B Rate" )           PORT_DIPLOCATION("DSW4:1,2,3")      /* OK - all other values are 2C/1C */
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, "Coin A Rate" )           PORT_DIPLOCATION("DSW4:4,5,6")      /* OK - all other values are 1C/1C, manual says 0x30 = 1C/100C but it doesn't work */
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x28, "1 Coin/10 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( mbstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_2) PORT_NAME("Increment top's 'S' (bet?)")  // Increment top's 'S' (looks like bet)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_5) PORT_NAME("Collect")  // Collect. Also used to blank the BK.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_7) PORT_NAME("Bookkeeping: Down")  // BK: down.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8) PORT_NAME("Bookkeeping: Set/Enter - Start")  // BK: enter. (start?)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_V) PORT_NAME("Coin 1") PORT_IMPULSE(2)    // coin 1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_B) PORT_NAME("Coin 2") PORT_IMPULSE(2)    // coin 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_N) PORT_NAME("Note")      // note acceptor
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_M) PORT_NAME("Remote")    // remote credits
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_L) PORT_NAME("Coin 3 (tokens)")    // coin 3 (tokens)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Operator Bookkeeping")  // operator bookkeeping
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Payout")  // payout?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Supervisor Bookkeeping")  // supervisor bookkeeping

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )  // enter in a different mode, and no attract.
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  // seems to accellerate the different mode.
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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


static INPUT_PORTS_START( magodds )
	PORT_START("IN0")   /* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Start")

	PORT_START("IN1")   /* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Start")

	PORT_START("IN2")   /* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")

	PORT_START("IN4")   /* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Coin A Rate" )       PORT_DIPLOCATION("DSW1:1,2,3")      /* OK */
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/30 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Key In Rate" )       PORT_DIPLOCATION("DSW1:4,5,6")      /* OK - aka Coin B */
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x38, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/60 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin C Rate" )       PORT_DIPLOCATION("DSW1:7,8")        /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Main Game Level" )   PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "+4 - 56%" )
	PORT_DIPSETTING(    0x01, "+3 - 60%" )
	PORT_DIPSETTING(    0x02, "+2 - 64%" )
	PORT_DIPSETTING(    0x03, "+1 - 68%" )
	PORT_DIPSETTING(    0x04, "0 - 72%" )
	PORT_DIPSETTING(    0x05, "-1 - 76%" )
	PORT_DIPSETTING(    0x06, "-2 - 80%" )
	PORT_DIPSETTING(    0x07, "-3 - 84%" )
	PORT_DIPNAME( 0x08, 0x08, "Nudity / Strip" )    PORT_DIPLOCATION("DSW2:4")          /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3") /* marked as DSW4 */
	PORT_DIPNAME( 0x03, 0x03, "Coin In Limit" )     PORT_DIPLOCATION("DSW4:1,2")        /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "Unlimited" )
	PORT_DIPNAME( 0x04, 0x00, "Hopper Switch" )     PORT_DIPLOCATION("DSW4:3")          /* not checked */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, "Coin D Rate" )       PORT_DIPLOCATION("DSW4:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, "1 Coin/10 Credits" )

	PORT_START("DSW4") /* marked as DSW3 */
	PORT_DIPNAME( 0x03, 0x00, "Hopper Limit" )      PORT_DIPLOCATION("DSW3:1,2")    /* not checked */
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )           PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPSETTING(    0x0c, "64" )
	PORT_DIPNAME( 0x70, 0x00, "Credit Limit" )      PORT_DIPLOCATION("DSW3:5,6,7")  /* OK */
	PORT_DIPSETTING(    0x70, "5000" )
	PORT_DIPSETTING(    0x60, "10000" )
	PORT_DIPSETTING(    0x50, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "90000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( schery97 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Display Pts/Ticket") /* undocumented - works when credited */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Note In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats") // doesn't work in v352c4

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )   PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8 (Hardest)" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0xc0, "24" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" )            PORT_DIPLOCATION("DSW2:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )            PORT_DIPLOCATION("DSW2:2")  /* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )                   PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )                 PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )    /* 16 in the manual */
	PORT_DIPSETTING(    0x03, "16" )    /* 24 in the manual */
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Points" )   PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x50, "20" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0x90, "50" )
	PORT_DIPSETTING(    0xa0, "60" )
	PORT_DIPSETTING(    0xb0, "75" )
	PORT_DIPSETTING(    0xc0, "80" )
	PORT_DIPSETTING(    0xd0, "100" )
	PORT_DIPSETTING(    0xe0, "200" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )                         PORT_DIPLOCATION("DSW4:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show In Confirm Screen" )                PORT_DIPLOCATION("DSW4:2")  /* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )   /* percentage in the manual */
	PORT_DIPSETTING(    0x02, "Percentage" )            /* level of difficulty in the manual */
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )    PORT_DIPLOCATION("DSW4:3")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )                    PORT_DIPLOCATION("DSW4:4")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )                  PORT_DIPLOCATION("DSW4:5")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )                  PORT_DIPLOCATION("DSW4:6,7,8")  /* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )                                   PORT_DIPLOCATION("DSW5:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	/* DIP switches 2 to 5 work only for version 3.51 and above */
	PORT_DIPNAME( 0x02, 0x00, "Limit Score of Each Game to Max 10x Bet or $5.00" )  PORT_DIPLOCATION("DSW5:2")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )       PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )      PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x00, "Play Remaining Score when No Credit" )               PORT_DIPLOCATION("DSW5:3")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )       PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )      PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x08, 0x00, "Reset Remaining Score to Zero" )                     PORT_DIPLOCATION("DSW5:4")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )       PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )      PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x10, 0x00, "Ticket Dispense from Score" )                        PORT_DIPLOCATION("DSW5:5")  /* OK */
	PORT_DIPSETTING(    0x00, "Use TDDD" )          PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "Use Interface" )     PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x20, 0x20, "Reel Speed (ver 2.3)" )                              PORT_DIPLOCATION("DSW5:6")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x00, "Talking (ver 2.1)" )                                 PORT_DIPLOCATION("DSW5:7")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x40, "Very Little (only sounds)" )
	PORT_DIPSETTING(    0x00, "Full (sounds & speech)" )
	PORT_DIPNAME( 0x80, 0x00, "Count Game to Issue Ticket" )                        PORT_DIPLOCATION("DSW5:8")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nfb96 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Display Pts/Ticket") /* undocumented - works when credited */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Note In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings / Confirm")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )   PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8 (Hardest)" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "10" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, "20" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "30" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x18, "40" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "8" )     PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x08, "16" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "24" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x18, "32" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "10" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, "20" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x80, "30" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0xc0, "40" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "8" )     PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x40, "16" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x80, "24" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0xc0, "32" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" )            PORT_DIPLOCATION("DSW2:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )            PORT_DIPLOCATION("DSW2:2")  /* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )                   PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )                 PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "10" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "20" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "30" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x01, "8" )     PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x02, "16" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x03, "24" )    PORT_CONDITION("DSW5",0x10,EQUALS,0x10)
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Points" )  PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x50, "20" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0x90, "50" )
	PORT_DIPSETTING(    0xa0, "60" )
	PORT_DIPSETTING(    0xb0, "75" )
	PORT_DIPSETTING(    0xc0, "80" )
	PORT_DIPSETTING(    0xd0, "100" )
	PORT_DIPSETTING(    0xe0, "200" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )                         PORT_DIPLOCATION("DSW4:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show In Confirm Screen" )                PORT_DIPLOCATION("DSW4:2")  /* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )   /* percentage in the manual */
	PORT_DIPSETTING(    0x02, "Percentage" )            /* level of difficulty in the manual */
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )    PORT_DIPLOCATION("DSW4:3")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )                    PORT_DIPLOCATION("DSW4:4")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )                  PORT_DIPLOCATION("DSW4:5")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )                  PORT_DIPLOCATION("DSW4:6,7,8")  /* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )                                   PORT_DIPLOCATION("DSW5:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Limit Score of Each Game to Max 10x Bet or $5.00" )  PORT_DIPLOCATION("DSW5:2")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )               PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )              PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "Unused when DSW4-5 OFF" )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "Unused when DSW4-5 OFF" )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x00, "Use Printer" )                                       PORT_DIPLOCATION("DSW5:3")  /* OK */
	PORT_DIPSETTING(    0x00, "No (Use TDDD)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Game Name & Odds Table" )                       PORT_DIPLOCATION("DSW5:4")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Min-Max Bet Type" )                                  PORT_DIPLOCATION("DSW5:5")  /* OK */
	PORT_DIPSETTING(    0x10, "Base 8" )
	PORT_DIPSETTING(    0x00, "Base 10" )
	PORT_DIPNAME( 0x20, 0x20, "Play Score when no point left" )                     PORT_DIPLOCATION("DSW5:6")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )       PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )      PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x20, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x40, 0x00, "Reset Remaining Score when Game Over" )              PORT_DIPLOCATION("DSW5:7")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )       PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )      PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x40, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )   PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x80, 0x00, "Advanced Count Game" )                               PORT_DIPLOCATION("DSW5:8")  /* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


/* Displays tkt info on screen but has no settings or hopper controls other than "Ticket Out By" DSW */
/* All marked as "Unknown" until a manual or more information is found */
static INPUT_PORTS_START( nfb96tx )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused coin switch */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Ticket In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused keyin? - causes counter errors */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused coin switch */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused keyout? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )   PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8 (Hardest)" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" )            PORT_DIPLOCATION("DSW2:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )            PORT_DIPLOCATION("DSW2:2")  /* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin In Rate" )              PORT_DIPLOCATION("DSW2:3")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    /* Nickel slots in manual */
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" ) /* Penny slots in manual */
	PORT_DIPNAME( 0x38, 0x10, "Ticket In Value" )           PORT_DIPLOCATION("DSW2:4,5,6") /* OK */
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x10, "25" )
	PORT_DIPSETTING(    0x18, "50" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x28, "125" )
	PORT_DIPSETTING(    0x30, "250" )
	PORT_DIPSETTING(    0x38, "500" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:7")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in other manuals */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Coin In Limit" )                 PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW3:5")      /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW3:6")      /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW3:7")      /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW3:8")      /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )                         PORT_DIPLOCATION("DSW4:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )                      PORT_DIPLOCATION("DSW4:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )    PORT_DIPLOCATION("DSW4:3")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )                    PORT_DIPLOCATION("DSW4:4")  /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )                      PORT_DIPLOCATION("DSW4:5")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )                      PORT_DIPLOCATION("DSW4:6")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )                      PORT_DIPLOCATION("DSW4:7")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )                      PORT_DIPLOCATION("DSW4:8")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSW5:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Ticket Out By" )             PORT_DIPLOCATION("DSW5:3")  /* OK */
	PORT_DIPSETTING(    0x00, "Interface" )
	PORT_DIPSETTING(    0x04, "Direct Drive" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:4")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:5")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:6")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:7")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW5:8")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( fb2010 ) // hit 'start1' to init NVRAM for first time
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused coin switch */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Ticket In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused keyin? - causes counter errors */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused coin switch */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused keyout? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double Up" ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spin" ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Odds / Title" ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show 'Game' Text" ) PORT_DIPLOCATION("DSW5:8") // causes corruption in D-Up game?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( roypok96 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note In")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Rate" )                 PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 - 90%" )
	PORT_DIPSETTING(    0x06, "Level 2 - 85%" )
	PORT_DIPSETTING(    0x05, "Level 3 - 80%" )
	PORT_DIPSETTING(    0x04, "Level 4 - 75%" )
	PORT_DIPSETTING(    0x03, "Level 5 - 70%" )
	PORT_DIPSETTING(    0x02, "Level 6 - 65%" )
	PORT_DIPSETTING(    0x01, "Level 7 - 60%" )
	PORT_DIPSETTING(    0x00, "Level 8 - 55%" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double Up Game" )            PORT_DIPLOCATION("DSW2:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Use Printer" )               PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, "No (TDDD)" ) /* (Ticket Dispenser Direct Drive) */
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) ) /* Use Auto Ticket Dispense only */
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )                   PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )                 PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )   PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )                         PORT_DIPLOCATION("DSW4:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show Coin In Limit" )                    PORT_DIPLOCATION("DSW4:2")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )    PORT_DIPLOCATION("DSW4:3")  /* not checked */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )                    PORT_DIPLOCATION("DSW4:4")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )                  PORT_DIPLOCATION("DSW4:5")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )                  PORT_DIPLOCATION("DSW4:6,7,8")  /* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	/* Only versions 3.6 and up make use of DSW5. */
	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( roypok96a )
	PORT_INCLUDE( roypok96 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "16" )

	/* DSW5 is under top board, 1-6 are unused and should be set to off. Switches 7 & 8 are adjustable without removing top board. */
	PORT_MODIFY("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Unused - leave off" )            PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Reset Remaining Score To Zero" ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Count Game To Issue Ticket" )    PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( pokonl97 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Big / Bet 10")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Play (Bet 1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Small / End")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Start / Draw")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Display Pts/Ticket") /* OK - works when credited */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note In")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )   PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 - 90%" )
	PORT_DIPSETTING(    0x06, "Level 2 - 85%" )
	PORT_DIPSETTING(    0x05, "Level 3 - 80%" )
	PORT_DIPSETTING(    0x04, "Level 4 - 75%" )
	PORT_DIPSETTING(    0x03, "Level 5 - 70%" )
	PORT_DIPSETTING(    0x02, "Level 6 - 65%" )
	PORT_DIPSETTING(    0x01, "Level 7 - 60%" )
	PORT_DIPSETTING(    0x00, "Level 8 - 55%" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x18, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x28, "60" )
	PORT_DIPSETTING(    0x30, "70" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Big Hands Frequency" )       PORT_DIPLOCATION("DSW1:7,8")    /* OK - hit frequency of 4/5-of a kind & flushes */
	PORT_DIPSETTING(    0x00, "Level 1 - Lowest" )
	PORT_DIPSETTING(    0x40, "Level 2" )
	PORT_DIPSETTING(    0x80, "Level 3" )
	PORT_DIPSETTING(    0xc0, "Level 4 - Highest" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )            PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Use Printer" )               PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, "No (TDDD)" ) /* (Ticket Dispenser Direct Drive) */
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) ) /* Use AUTO TKT DISPENSE only */
	PORT_DIPNAME( 0x1c, 0x10, "Coin In Rate" )              PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )             PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )   PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )             PORT_DIPLOCATION("DSW4:1")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Type of Game" )              PORT_DIPLOCATION("DSW4:3")  /* OK */
	PORT_DIPSETTING(    0x00, "Discard" )
	PORT_DIPSETTING(    0x04, "Hold" )
	PORT_DIPNAME( 0x08, 0x08, "Odds Table" )                PORT_DIPLOCATION("DSW4:4")  /* OK */
	PORT_DIPSETTING(    0x08, "Table 1" )
	PORT_DIPSETTING(    0x00, "Table 2" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )      PORT_DIPLOCATION("DSW4:5")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )      PORT_DIPLOCATION("DSW4:6,7,8")
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:1")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Use Printer" )                   PORT_DIPLOCATION("DSW5:3")  /* OK */
	PORT_DIPSETTING(    0x00, "No (Use TDDD)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Game Name & Odds Table" )   PORT_DIPLOCATION("DSW5:4")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:5")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:6")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:7")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )               PORT_DIPLOCATION("DSW5:8")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( match98 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_C) PORT_NAME("Hit / Big / Stop")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Auto Hit / Double")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_V) PORT_NAME("Small")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note In")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )   PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x18, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x28, "60" )
	PORT_DIPSETTING(    0x30, "80" )
	PORT_DIPSETTING(    0x38, "100" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xc0, "30" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:1")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )                   PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )                 PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")  /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )   PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:!1") /* unknown */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:!2") /* unknown */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:!3") /* unknown */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:!4") /* unknown */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )      PORT_DIPLOCATION("DSW4:!5") /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )      PORT_DIPLOCATION("DSW4:!6,!7,!8") /* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nfb96bl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")       /* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)            /* Coin A */

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Rate" )                 PORT_DIPLOCATION("DSW1:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "Level 1 - 90%" )
	PORT_DIPSETTING(    0x06, "Level 2 - 85%" )
	PORT_DIPSETTING(    0x05, "Level 3 - 80%" )
	PORT_DIPSETTING(    0x04, "Level 4 - 75%" )
	PORT_DIPSETTING(    0x03, "Level 5 - 70%" )
	PORT_DIPSETTING(    0x02, "Level 6 - 65%" )
	PORT_DIPSETTING(    0x01, "Level 7 - 60%" )
	PORT_DIPSETTING(    0x00, "Level 8 - 55%" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )              PORT_DIPLOCATION("DSW1:4,5,6")  /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )    PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )            PORT_DIPLOCATION("DSW2:1")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )            PORT_DIPLOCATION("DSW2:2")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )                   PORT_DIPLOCATION("DSW2:3,4,5")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In Value" )             PORT_DIPLOCATION("DSW2:6,7")    /* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )       PORT_DIPLOCATION("DSW2:8")      /* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )         PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Points" )  PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )           PORT_DIPLOCATION("DSW3:5,6,7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )                         PORT_DIPLOCATION("DSW4:1")      /* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show Coin In Limit On Screen" )          PORT_DIPLOCATION("DSW4:2")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )    PORT_DIPLOCATION("DSW4:3")      /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )                    PORT_DIPLOCATION("DSW4:4")      /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )                  PORT_DIPLOCATION("DSW4:5")      /* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x00, "Ticket Dispense Mode" )                  PORT_DIPLOCATION("DSW4:6,7")    /* OK */
	PORT_DIPSETTING(    0x60, "Continuous" )
	PORT_DIPSETTING(    0x40, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 5 Tickets Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Tickets Per Game" )
	PORT_DIPNAME( 0x80, 0x00, "Show In Confirm Screen" )                PORT_DIPLOCATION("DSW4:8")      /* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )
	PORT_DIPSETTING(    0x80, "Percentage" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* no manual - similiar to nfb96 sets */
static INPUT_PORTS_START( nfm )
	PORT_INCLUDE( nfb96bl )

	PORT_MODIFY( "IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                           /* tied to hopper somehow?  fill/empty switch? */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )                           /* display ticket value? */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )                           /* keyin?  tied to ticket clear value */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")    /* DSW4-1 must be on to access account menu */

	PORT_MODIFY( "DSW2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:2")  /* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Spin Length" )           PORT_DIPLOCATION("DSW2:8")  /* OK */
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPSETTING(    0x80, "Short" )
INPUT_PORTS_END


static INPUT_PORTS_START( unkch_controls )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Bet A / Stop 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Cash Out")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Take / Stop 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet B / D-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Small / Stop 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_CODE(KEYCODE_B) PORT_NAME("Big")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  /* Trips "call attendant" state if activated while credited - something to do with hopper out? */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("tickets", ticket_dispenser_device, line_r)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
INPUT_PORTS_END

static INPUT_PORTS_START( unkch )
	PORT_INCLUDE( unkch_controls )

	/* Like many of the other games on this hardware, there is an
	input & dip test screen that you can stumble in to. Also a picture viewer option. Can't figure
	out exactly how to make it repeatable... */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Level" )                PORT_DIPLOCATION("DSW1:1,2")        /* OK */
	PORT_DIPSETTING(    0x03, "Easy" )
	PORT_DIPSETTING(    0x02, "Mid 1" )
	PORT_DIPSETTING(    0x01, "Mid 2" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x04, 0x04, "Punti" )                     PORT_DIPLOCATION("DSW1:3")          /* OK */
	PORT_DIPSETTING(    0x04, "Ticket" )
	PORT_DIPSETTING(    0x00, "Gettoni" )
	PORT_DIPNAME( 0x08, 0x08, "Ticket Dispenser" )          PORT_DIPLOCATION("DSW1:4")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Reel Speed" )                PORT_DIPLOCATION("DSW1:5")          /* OK */
	PORT_DIPSETTING(    0x10, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Jackpot" )             PORT_DIPLOCATION("DSW1:6,7")        /* OK */
	PORT_DIPSETTING(    0x60, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, "5%" )
	PORT_DIPSETTING(    0x00, "10%" )
	PORT_DIPSETTING(    0x40, "20%" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Step On 8" )             PORT_DIPLOCATION("DSW1:8")          /* OK */
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, "Main/Bonus Game Rate" )      PORT_DIPLOCATION("DSW2:1,2,3,4")    /* OK - all other values are 83% / 88% */
	PORT_DIPSETTING(    0x00, "50% / 55%" )
	PORT_DIPSETTING(    0x01, "53% / 58%" )
	PORT_DIPSETTING(    0x02, "56% / 61%" )
	PORT_DIPSETTING(    0x03, "59% / 64%" )
	PORT_DIPSETTING(    0x04, "62% / 67%" )
	PORT_DIPSETTING(    0x05, "65% / 70%" )
	PORT_DIPSETTING(    0x06, "68% / 73%" )
	PORT_DIPSETTING(    0x07, "71% / 76%" )
	PORT_DIPSETTING(    0x08, "74% / 79%" )
	PORT_DIPSETTING(    0x09, "77% / 82%" )
	PORT_DIPSETTING(    0x0a, "80% / 85%" )
	PORT_DIPSETTING(    0x0f, "83% / 88%" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )                   PORT_DIPLOCATION("DSW2:5,6")        /* OK */
	PORT_DIPSETTING(    0x00, "10 (5)" )    PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 5 in settings screen but limits at 10 in gameplay */
	PORT_DIPSETTING(    0x10, "20 (10)" )   PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 10 in settings screen but limits at 20 in gameplay */
	PORT_DIPSETTING(    0x20, "40 (20)" )   PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 20 in settings screen but limits at 40 in gameplay */
	PORT_DIPSETTING(    0x00, "5" )         PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "10" )        PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "20" )        PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "64" )                                                /* always individual irrespective of DSW3-8 */
	PORT_DIPNAME( 0x40, 0x40, "Min. Bet For Bonus Play" )   PORT_DIPLOCATION("DSW2:7")          /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Coin A Rate" )               PORT_DIPLOCATION("DSW3:1,2")        /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x04, 0x00, "Gettoni/Ticket" )            PORT_DIPLOCATION("DSW3:3")          /* OK */
	PORT_DIPSETTING(    0x00, "10/100" )
	PORT_DIPSETTING(    0x04, "20/200" )
	PORT_DIPNAME( 0x18, 0x00, "Key In Rate" )               PORT_DIPLOCATION("DSW3:4,5")        /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/25 Credits" )     PORT_CONDITION("DSW3",0x03,EQUALS,0x00) /* 5*5 */
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )     PORT_CONDITION("DSW3",0x03,EQUALS,0x00) /* 5*10 */
	PORT_DIPSETTING(    0x10, "1 Coin/100 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x00) /* 5*20 */
	PORT_DIPSETTING(    0x18, "1 Coin/250 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x00) /* 5*50 */
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )     PORT_CONDITION("DSW3",0x03,EQUALS,0x01) /* 10*5 */
	PORT_DIPSETTING(    0x08, "1 Coin/100 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x01) /* 10*10 */
	PORT_DIPSETTING(    0x10, "1 Coin/200 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x01) /* 10*20 */
	PORT_DIPSETTING(    0x18, "1 Coin/500 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x01) /* 10*50 */
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x02) /* 20*5 */
	PORT_DIPSETTING(    0x08, "1 Coin/200 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x02) /* 20*10 */
	PORT_DIPSETTING(    0x10, "1 Coin/400 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x02) /* 20*20 */
	PORT_DIPSETTING(    0x18, "1 Coin/1,000 Credits" )  PORT_CONDITION("DSW3",0x03,EQUALS,0x02) /* 20*50 */
	PORT_DIPSETTING(    0x00, "1 Coin/250 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x03) /* 50*5 */
	PORT_DIPSETTING(    0x08, "1 Coin/500 Credits" )    PORT_CONDITION("DSW3",0x03,EQUALS,0x03) /* 50*10 */
	PORT_DIPSETTING(    0x10, "1 Coin/1,000 Credits" )  PORT_CONDITION("DSW3",0x03,EQUALS,0x03) /* 50*20 */
	PORT_DIPSETTING(    0x18, "1 Coin/2,500 Credits" )  PORT_CONDITION("DSW3",0x03,EQUALS,0x03) /* 50*50 */
	PORT_DIPNAME( 0x20, 0x00, "Coin B Enable" )             PORT_DIPLOCATION("DSW3:6")          /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Coin B Rate" )               PORT_DIPLOCATION("DSW3:7")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x00, "Max Bet Type" )              PORT_DIPLOCATION("DSW3:8")          /* OK */
	PORT_DIPSETTING(    0x80, "Total" )        /* Max Bet applies to total of BET-A and BET-B unless set to 64 */
	PORT_DIPSETTING(    0x00, "Individual" )   /* Max Bet applies individually to each of BET-A and BET-B */

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Super Jackpot Half" )        PORT_DIPLOCATION("DSW4:4")          /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Red Game Credit" )           PORT_DIPLOCATION("DSW4:6,7")        /* OK */
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x60, "20" )
	PORT_DIPNAME( 0x80, 0x80, "Cherry/Bell Bonus" )         PORT_DIPLOCATION("DSW4:8")          /* OK */
	PORT_DIPSETTING(    0x80, "x6 / x3" )
	PORT_DIPSETTING(    0x00, "x9 / x5" )
INPUT_PORTS_END

static INPUT_PORTS_START( unkch3 )
	PORT_INCLUDE( unkch_controls )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Level" )                PORT_DIPLOCATION("DSW1:1,2")    /* OK */
	PORT_DIPSETTING(    0x03, "Easy" )
	PORT_DIPSETTING(    0x02, "Mid 1" )
	PORT_DIPSETTING(    0x01, "Mid 2" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x04, 0x04, "Punti Unit" )                PORT_DIPLOCATION("DSW1:3")      /* OK */
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPNAME( 0x08, 0x08, "Ticket Dispenser" )          PORT_DIPLOCATION("DSW1:4")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Reel Speed" )                PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )                   PORT_DIPLOCATION("DSW2:5,6")        /* OK */
	PORT_DIPSETTING(    0x00, "10 (5)" )    PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 5 in settings screen but limits at 10 in gameplay */
	PORT_DIPSETTING(    0x10, "20 (10)" )   PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 10 in settings screen but limits at 20 in gameplay */
	PORT_DIPSETTING(    0x20, "40 (20)" )   PORT_CONDITION("DSW3",0x80,EQUALS,0x80) /* shows 20 in settings screen but limits at 40 in gameplay */
	PORT_DIPSETTING(    0x00, "5" )         PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "10" )        PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "20" )        PORT_CONDITION("DSW3",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "64" )                                                /* always individual irrespective of DSW3-8 */
	PORT_DIPNAME( 0x40, 0x40, "Min. Bet For Bonus Play" )   PORT_DIPLOCATION("DSW2:7")          /* OK - called 'Bet Minimum' in settings screen */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown) )           PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin B Enable" )             PORT_DIPLOCATION("DSW3:6")      /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Coin B Rate" )               PORT_DIPLOCATION("DSW3:7")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x00, "Max Bet Type" )              PORT_DIPLOCATION("DSW3:8")      /* OK */
	PORT_DIPSETTING(    0x80, "Total" )        /* Max Bet applies to total of BET-A and BET-B unless set to 64 */
	PORT_DIPSETTING(    0x00, "Individual" )   /* Max Bet applies individually to each of BET-A and BET-B */

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Test Mode" )                 PORT_DIPLOCATION("DSW4:5")      /* OK */
	PORT_DIPSETTING(    0x10, "Disable" )
	PORT_DIPSETTING(    0x00, "Enable" )    /* hold 'Settings' on reset to access */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Cherry/Bell Bonus" )         PORT_DIPLOCATION("DSW4:8")      /* OK */
	PORT_DIPSETTING(    0x80, "x6 / x3" )
	PORT_DIPSETTING(    0x00, "x9 / x5" )
INPUT_PORTS_END

static INPUT_PORTS_START( unkch4 )
	PORT_INCLUDE( unkch_controls )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C")

	/* Like many of the other games on this hardware, there is an
	input & dip test screen that you can stumble in to. Also a picture viewer option. Can't figure
	out exactly how to make it repeatable... */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Level" )                PORT_DIPLOCATION("DSW1:1,2")        /* OK */
	PORT_DIPSETTING(    0x03, "Easy" )
	PORT_DIPSETTING(    0x02, "Mid 1" )
	PORT_DIPSETTING(    0x01, "Mid 2" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x04, 0x04, "Punti" )                     PORT_DIPLOCATION("DSW1:3")          /* OK */
	PORT_DIPSETTING(    0x04, "Ticket" )    /* payout rate 100 */
	PORT_DIPSETTING(    0x00, "Gettoni" )   /* payout rate 10 */
	PORT_DIPNAME( 0x08, 0x08, "Ticket Dispenser" )          PORT_DIPLOCATION("DSW1:4")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Reel Speed" )                PORT_DIPLOCATION("DSW1:5")          /* OK */
	PORT_DIPSETTING(    0x10, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Jackpot" )             PORT_DIPLOCATION("DSW1:6,7")        /* shows in test mode but always seems to be enabled in gameplay */
	PORT_DIPSETTING(    0x60, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, "5%" )
	PORT_DIPSETTING(    0x00, "10%" )
	PORT_DIPSETTING(    0x40, "20%" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Step On 8" )             PORT_DIPLOCATION("DSW1:8")          /* OK */
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, "Main/Bonus Game Rate" )      PORT_DIPLOCATION("DSW2:1,2,3,4")    /* OK - all other values are 83% / 88% */
	PORT_DIPSETTING(    0x00, "50% / 55%" )
	PORT_DIPSETTING(    0x01, "53% / 58%" )
	PORT_DIPSETTING(    0x02, "56% / 61%" )
	PORT_DIPSETTING(    0x03, "59% / 64%" )
	PORT_DIPSETTING(    0x04, "62% / 67%" )
	PORT_DIPSETTING(    0x05, "65% / 70%" )
	PORT_DIPSETTING(    0x06, "68% / 73%" )
	PORT_DIPSETTING(    0x07, "71% / 76%" )
	PORT_DIPSETTING(    0x08, "74% / 79%" )
	PORT_DIPSETTING(    0x09, "77% / 82%" )
	PORT_DIPSETTING(    0x0a, "80% / 85%" )
	PORT_DIPSETTING(    0x0f, "83% / 88%" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )                   PORT_DIPLOCATION("DSW2:5,6")        /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x20, "32" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPNAME( 0x40, 0x40, "Min. Bet For Bonus Play" )   PORT_DIPLOCATION("DSW2:7")          /* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x00, "Coin A Rate" )               PORT_DIPLOCATION("DSW3:1,2,3")      /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x05, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x18, 0x00, "Key In Rate" )               PORT_DIPLOCATION("DSW3:4,5")        /* OK */
	PORT_DIPSETTING(    0x00, "5x Coin A" )
	PORT_DIPSETTING(    0x08, "10x Coin A" )
	PORT_DIPSETTING(    0x10, "20x Coin A" )
	PORT_DIPSETTING(    0x18, "50x Coin A" )
	PORT_DIPNAME( 0x20, 0x00, "Coin C Rate" )               PORT_DIPLOCATION("DSW3:6")          /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )        PORT_CONDITION("DSW3",0x07,EQUALS,0x00) /* 1*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x00) /* 1*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x01) /* 2*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/20 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x01) /* 2*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/20 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x02) /* 4*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/40 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x02) /* 4*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/25 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x03) /* 5*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/50 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x03) /* 5*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/40 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x04) /* 8*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/80 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x04) /* 8*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )     PORT_CONDITION("DSW3",0x07,EQUALS,0x05) /* 10*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/100 Credits" )    PORT_CONDITION("DSW3",0x07,EQUALS,0x05) /* 10*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )    PORT_CONDITION("DSW3",0x07,EQUALS,0x06) /* 20*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/200 Credits" )    PORT_CONDITION("DSW3",0x07,EQUALS,0x06) /* 20*10 */
	PORT_DIPSETTING(    0x00, "1 Coin/250 Credits" )    PORT_CONDITION("DSW3",0x07,EQUALS,0x07) /* 50*5 */
	PORT_DIPSETTING(    0x20, "1 Coin/500 Credits" )    PORT_CONDITION("DSW3",0x07,EQUALS,0x07) /* 50*10 */
	PORT_DIPNAME( 0x40, 0x00, "Coin B Rate" )               PORT_DIPLOCATION("DSW3:7")          /* OK */
	PORT_DIPSETTING(    0x00, "1x Coin C" )
	PORT_DIPSETTING(    0x40, "2x Coin C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Cherry/Bell Bonus" )         PORT_DIPLOCATION("DSW4:8")          /* OK */
	PORT_DIPSETTING(    0x80, "x6 / x3" )
	PORT_DIPSETTING(    0x00, "x9 / x5" )
INPUT_PORTS_END


static INPUT_PORTS_START( magoddsc )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4")
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

static INPUT_PORTS_START( megaline )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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


static INPUT_PORTS_START( bonusch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "DSW5" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

static INPUT_PORTS_START( feverch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7") // if low "check voltage call attendant" message will appear
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
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
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

static INPUT_PORTS_START( star100 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Hold / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

//  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
//  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3: BIG")
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4: DOUBLE UP")
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5: TAKE")
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6: BET")
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7: SMALL")
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8: START")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Limited Payout")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings / Test Mode") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("IN3")   // reflected in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   // reflected in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Key Out" )               PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x01, "x100" )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Rate" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "1/24" )
	PORT_DIPSETTING(    0x02, "1/32" )
	PORT_DIPNAME( 0x04, 0x04, "Spin Rate" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x18, 0x18, "Double Up Rate" )        PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0xe0, 0xe0, "Game Rate" )             PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0xe0, "50%" )
	PORT_DIPSETTING(    0xc0, "60%" )
	PORT_DIPSETTING(    0xa0, "65%" )
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0x60, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x20, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Min Bet" )         PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x02, 0x02, "Number of Jackpot" )     PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "3 2" )
	PORT_DIPSETTING(    0x02, "6 3" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up" )             PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x18, 0x18, "Payout Limit" )          PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1000" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus" )                 PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_DIPSETTING(    0x00, "Random" )
	PORT_DIPNAME( 0xc0, 0xc0, "Number of Clown" )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "60%" )
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Rate" )           PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "100 Credits" )
	PORT_DIPSETTING(    0x01, "110 Credits" )
	PORT_DIPSETTING(    0x02, "120 Credits" )
	PORT_DIPSETTING(    0x03, "130 Credits" )
	PORT_DIPSETTING(    0x04, "200 Credits" )
	PORT_DIPSETTING(    0x05, "400 Credits" )
	PORT_DIPSETTING(    0x06, "500 Credits" )
	PORT_DIPSETTING(    0x07, "1000 Credits" )
	PORT_DIPNAME( 0x18, 0x00, "Bet Limit" )             PORT_DIPLOCATION("SW3:4,5")
	PORT_DIPSETTING(    0x18, "32 (Limit of Bonus: 1/4)" )
	PORT_DIPSETTING(    0x10, "64 (Limit of Bonus: 1/2)" )
	PORT_DIPSETTING(    0x08, "72 (Limit of Bonus: All)" )
	PORT_DIPSETTING(    0x00, "80 (Limit of Bonus: All)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4-0")
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )      PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )

	/* the following two are 'bonus', and need conditional port since they are in different banks */
	PORT_DIPNAME( 0x08, 0x08, "Bonus (switch-1)" )          PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "20000" )    PORT_CONDITION("DSW4-1", 0x01, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "40000" )    PORT_CONDITION("DSW4-1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "30000" )    PORT_CONDITION("DSW4-1", 0x01, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "50000" )    PORT_CONDITION("DSW4-1", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4-1")
	PORT_DIPNAME( 0x01, 0x01, "Bonus (switch-2)" )          PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Max Bonus" )                 PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPSETTING(    0x02, "200000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" )               PORT_DIPLOCATION("SW4:7,8")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW5")  // physically tied to ay8910 port A, but unused...
	PORT_DIPNAME( 0x01, 0x01, "DSW5" )                  PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW6")  // physically tied to ay8910 port B, but unused...
	PORT_DIPNAME( 0x01, 0x01, "DSW6" )                  PORT_DIPLOCATION("SW6:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW6:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( crazybon )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Hold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )    /* also works as stop all and hold pair */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_INCLUDE( cmv4_coins )
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )

	PORT_INCLUDE( cmv4_service )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Hold Pair" )                 PORT_DIPLOCATION("DSW1:1")      /* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Double Up Jack" )            PORT_DIPLOCATION("DSW1:2")      /* from manual, not checked */
	PORT_DIPSETTING(    0x02, "Poker" )
	PORT_DIPSETTING(    0x00, "Missile" )
	PORT_DIPNAME( 0x04, 0x00, "Coin Out Rate" )             PORT_DIPLOCATION("DSW1:3")      /* OK */
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "10" )    PORT_CONDITION("DSW3-0",0x03,EQUALS,0x03)
	PORT_DIPSETTING(    0x00, "50" )    PORT_CONDITION("DSW3-0",0x03,EQUALS,0x02)
	PORT_DIPSETTING(    0x00, "100" )   PORT_CONDITION("DSW3-0",0x03,EQUALS,0x01)
	PORT_DIPSETTING(    0x00, "500" )   PORT_CONDITION("DSW3-0",0x03,EQUALS,0x00)
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:4")      /* OK */
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPSETTING(    0x00, "Win" )
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:5")      /* OK */
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Game" )            PORT_DIPLOCATION("DSW1:6")      /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Max Bet" )                   PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0xc0, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0x40, "64" )
	PORT_DIPSETTING(    0x00, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )        PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Hopper Limit" )              PORT_DIPLOCATION("DSW2:4,5")   /* not checked */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "Mode" )                      PORT_DIPLOCATION("DSW2:6")     /* OK */
	PORT_DIPSETTING(    0x00, "Game" )
	PORT_DIPSETTING(    0x20, "Stealth" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3-0")
	PORT_DIPNAME( 0x03, 0x02, "Key In Rate" )               PORT_DIPLOCATION("DSW3:1,2")    /* OK */
	PORT_DIPSETTING(    0x03, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPNAME( 0x0c, 0x00, "Coin A Rate" )               PORT_DIPLOCATION("DSW3:3,4")    /* OK */
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3-1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Lock Into Stealth Mode" )    PORT_DIPLOCATION("DSW3:8")      /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   /* prevents switching to game mode with start/bet buttons */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x01, "Credit Limit" )              PORT_DIPLOCATION("DSW4:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Limit Credit" )              PORT_DIPLOCATION("DSW4:4")      /* from manual, not checked */
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Fast Take With" )            PORT_DIPLOCATION("DSW4:5")      /* OK */
	PORT_DIPSETTING(    0x10, "Take" )
	PORT_DIPSETTING(    0x00, "Start" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Min Bet" )             PORT_DIPLOCATION("DSW4:6")      /* OK */
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )                PORT_DIPLOCATION("DSW4:7")      /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "C.B" )                       PORT_DIPLOCATION("DSW4:8")      /* from manual, not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Print" )                     PORT_DIPLOCATION("DSW5:1")      /* from manual, not checked */
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x06, "Max Key In" )                PORT_DIPLOCATION("DSW5:2,3")    /* OK */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x00, "Condition For 3 Fruit Bonus" )    PORT_DIPLOCATION("DSW5:4,5")    /* OK */
	PORT_DIPSETTING(    0x18, "5-<-7" )     /* don't know what "<" means, but that's what the game displays */
	PORT_DIPSETTING(    0x10, "5-9-5" )
	PORT_DIPSETTING(    0x08, "5-6-3" )
	PORT_DIPSETTING(    0x00, "5-3-2" )
	PORT_DIPNAME( 0x60, 0x20, "Game Min Bet" )              PORT_DIPLOCATION("DSW5:6,7")    /* OK */
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x80, "Card Shuffle Animation" )    PORT_DIPLOCATION("DSW5:8")      /* OK */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmpacman )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )    PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_UP)    PORT_NAME("Stop 2 / Big / Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )    PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Stop 1 / D-UP / Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("Stop All / Take / Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )    PORT_CODE(KEYCODE_V)                          PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )        PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Start / Right")

	PORT_INCLUDE( cmv4_coins )
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )         PORT_CODE(KEYCODE_K) PORT_NAME("Hidden switch for change games")

	PORT_INCLUDE( cmv4_service )
	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* Tied to GND and to the hidden switch that change games. (PC0+GND) -+-> PB0 */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Unknown" )                   PORT_DIPLOCATION("DSW1:!1")     /* not checked */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )         PORT_DIPLOCATION("DSW1:!2")     /* not checked */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )               PORT_DIPLOCATION("DSW1:!3")     /* not checked */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "'7' In Double Up Game" )     PORT_DIPLOCATION("DSW1:!4")     /* OK */
	PORT_DIPSETTING(    0x00, "Lose" )
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "Double Up Game Pay Rate" )   PORT_DIPLOCATION("DSW1:!5")     /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Game" )            PORT_DIPLOCATION("DSW1:!6")     /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )                   PORT_DIPLOCATION("DSW1:!7,!8")  /* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:!1,!2,!3")   /* OK */
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:!4,!5")      /* not checked */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:!6")         /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )           PORT_DIPLOCATION("DSW2:!7")         /* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )  PORT_DIPLOCATION("DSW2:!8")     /* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:!1,!2")  /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:!3,!4")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:!5,!6")  /* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:!7,!8")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_INCLUDE( cmv4_dsw4 )   /* Display Of Payout Limit not working; all others OK */

	PORT_INCLUDE( cmv4_dsw5 )
	PORT_MODIFY("DSW5")
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus OK */
	/* Display Of Doll At All Fr. Bonus not checked */
	PORT_DIPNAME( 0x40, 0x40, "Card Shuffle Animation" )    PORT_DIPLOCATION("DSW5:7")  /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Test Mode For Disp. Of Doll not working */
INPUT_PORTS_END


static INPUT_PORTS_START( cmtetris )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )    PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Stop 2 / Big / Right")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )    PORT_CODE(KEYCODE_X)                         PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_UP)   PORT_NAME("Stop All / Take / Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )    PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Stop 3 / Small / Info / Left")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )        PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Start / Down")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )         PORT_CODE(KEYCODE_K) PORT_NAME("Hidden switch for Cherry Master")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )         PORT_CODE(KEYCODE_L) PORT_NAME("Hidden switch for Tetris")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B") /* Coin Service (tied to PPI u34 PB3 to also coin Tetris game) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME("Coin D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME("Coin C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")

	PORT_INCLUDE( cmv4_service )
	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* Tied to GND and to the hidden switch that change games. (PC0+GND) -+-> (PB0|PB1) */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:!1")  /* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" ) PORT_DIPLOCATION("DSW1:!2")  /* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )       PORT_DIPLOCATION("DSW1:!3")  /* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )          PORT_DIPLOCATION("DSW1:!4")  /* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )     PORT_DIPLOCATION("DSW1:!5")  /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )         PORT_DIPLOCATION("DSW1:!6")  /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )           PORT_DIPLOCATION("DSW1:!7,!8")    /* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:!1,!2,!3")  /* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:!4,!5")    /* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:!6")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )           PORT_DIPLOCATION("DSW2:!7")  /* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )  PORT_DIPLOCATION("DSW2:!8")  /* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_INCLUDE( cmv4_dsw3 )   /* all OK */

	PORT_INCLUDE( cmv4_dsw4 )   /* Display Of Payout Limit not working; all others OK */

	PORT_INCLUDE( cmv4_dsw5 )
	/* Display of Doll On Demo only affects payout table screen */
	/* Coin In Limit OK */
	/* Condition For 3 Kind Of Bonus not checked */
	/* Display Of Doll At All Fr. Bonus not checked */
	/* DSW5-7 listed as unused */
	/* Test Mode For Disp. Of Doll not working */
INPUT_PORTS_END


static INPUT_PORTS_START( flam7_w4 )
	PORT_START("IN0")   /* b800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Button Lockout")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 - Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 - Low")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Start")

	PORT_START("IN1")   /* b801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* b802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER( "fl7w4_id", ds2401_device, read )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* b810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Main Door SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Change")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Logic Door SW")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Cash Door SW")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")  // confirmed.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")  // confirmed.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Coin C or Mars")  // confirmed.

	PORT_START("IN4")   /* b811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)   PORT_NAME("WT RXD")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)   PORT_NAME("COUT RTS")  // related to hopper...
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)   PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)   PORT_NAME("IN4-4 Active")  // This one is active in real PCB.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)   PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Books / Stats / Setup") PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Payout" )               PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "Amusement (no credits out)" )
	PORT_DIPSETTING(    0x02, "Ticket Printer" )
	PORT_DIPSETTING(    0x01, "Hopper Payout" )
	PORT_DIPSETTING(    0x00, "Remote Clear" )
	PORT_DIPNAME( 0x04, 0x04, "Game Speed" )           PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Fast Game" )
	PORT_DIPSETTING(    0x00, "Slow Game" )
	PORT_DIPNAME( 0x08, 0x08, "Double-Up" )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Graphics Type" )        PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "Regular Fruit" )
	PORT_DIPSETTING(    0x20, "Graphics Option 1" )  // Numbers.
	PORT_DIPSETTING(    0x10, "Graphics Option 2" )  // Red, White & Blue 7's.
	PORT_DIPSETTING(    0x00, "Graphics Option 3" )  // Hollywood Nights.
	PORT_DIPNAME( 0xc0, 0xc0, "Paytable" )             PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "9322" )
	PORT_DIPSETTING(    0x80, "9323" )
	PORT_DIPSETTING(    0x40, "9324" )
	PORT_DIPSETTING(    0x00, "9321" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Credits Per Coin A" )   PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x01, "25" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x38, 0x38, "Credits Per Coin B" )   PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "25" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0xc0, 0xc0, "Credits Per Bill" )     PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "100" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Maximum Bet" )          PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x01, "32" )
	PORT_DIPSETTING(    0x00, "32" )  // confirmed.
	PORT_DIPNAME( 0x04, 0x04, "Coin-In Timeout" )      PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Denomination" )         PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(    0x18, "50 Cents" )
	PORT_DIPSETTING(    0x10, "25 Cents" )
	PORT_DIPSETTING(    0x08, "10 Cents" )
	PORT_DIPSETTING(    0x00, "5 Cents" )
	PORT_DIPNAME( 0x60, 0x60, "Progressive Sign" )     PORT_DIPLOCATION("DSW3:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "Single Level" )
	PORT_DIPSETTING(    0x20, "Multi Level" )
	PORT_DIPSETTING(    0x00, "Test Mode" )
	PORT_DIPNAME( 0x80, 0x80, "DSW3_80 Unknown" )      PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Button Lockout" )       PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Lockout Polarity" )     PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPNAME( 0x0c, 0x0c, "Printer Type" )         PORT_DIPLOCATION("DSW4:3,4")
	PORT_DIPSETTING(    0x0c, "Epson 267A" )
	PORT_DIPSETTING(    0x08, "Star 300" )
	PORT_DIPSETTING(    0x04, "Ithaca 76" )
	PORT_DIPSETTING(    0x00, "Spare" )
	PORT_DIPNAME( 0x10, 0x10, "DSW4_10 Unknown" )      PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW4_20 Unknown" )      PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW4_40 Unknown" )      PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW4_80 Unknown" )      PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static INPUT_PORTS_START( flaming7 )
	PORT_START("IN0")   /* b800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Button Lockout")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 - Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 - Low")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Start")

	PORT_START("IN1")   /* b801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* b802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER( "fl7w4_id", ds2401_device, read )  // Same input, different device.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* b810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Main Door SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Change")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Logic Door SW")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Cash Door SW")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")  // confirmed.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")  // confirmed.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Coin C or Mars")  // confirmed.

	PORT_START("IN4")   /* b811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)   PORT_NAME("WT RXD")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)   PORT_NAME("COUT RTS")  // related to hopper...
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)   PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)   PORT_NAME("IN4-4 Active")  // This one is active in real PCB.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)   PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Books / Stats / Setup") PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Credits Out" )          PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "Amusement (no credits out)" )
	PORT_DIPSETTING(    0x02, "Ticket Printer" )
	PORT_DIPSETTING(    0x01, "Hopper Payout" )
	PORT_DIPSETTING(    0x00, "Remote Clear" )
	PORT_DIPNAME( 0x04, 0x04, "Game Speed" )           PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Fast Game" )
	PORT_DIPSETTING(    0x00, "Slow Game" )
	PORT_DIPNAME( 0x08, 0x08, "Lock-Up on 7 or 8 of a kind" )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Graphics Type" )        PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "Regular Fruit" )
	PORT_DIPSETTING(    0x20, "Numbers" )
	PORT_DIPSETTING(    0x10, "Graphics Option 1" )
	PORT_DIPSETTING(    0x00, "Graphics Option 2" )
	PORT_DIPNAME( 0xc0, 0xc0, "Paytable" )             PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "9325 (92.50%)" )
	PORT_DIPSETTING(    0x80, "9326 (90.45%)" )
	PORT_DIPSETTING(    0x40, "9327 (90.53%)" )
	PORT_DIPSETTING(    0x00, "9328 (90.60%)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Bonus Pay" )   PORT_DIPLOCATION("DSW2:1,2,3")  // percentage of how quickly the internal progressive advances.
	PORT_DIPSETTING(    0x07, "1.00%" )
	PORT_DIPSETTING(    0x06, "2.00%" )
	PORT_DIPSETTING(    0x05, "3.30%" )
	PORT_DIPSETTING(    0x04, "4.00%" )
	PORT_DIPSETTING(    0x03, "5.00%" )
	PORT_DIPSETTING(    0x02, "6.20%" )
	PORT_DIPSETTING(    0x01, "8.30%" )
	PORT_DIPSETTING(    0x00, "10%" )
	PORT_DIPNAME( 0x08, 0x00, "Internal Progressive" )   PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Not Used (Leave OFF)" )   PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Not Used (Leave OFF)" )   PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Not Used (Leave OFF)" )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Not Used (Leave OFF)" )   PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Maximum Bet" )          PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x01, "32" )
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPNAME( 0x04, 0x04, "Coin-In Timeout" )      PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Denomination" )         PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x38, "$1.00" )
	PORT_DIPSETTING(    0x30, "50 Cents" )
	PORT_DIPSETTING(    0x28, "25 Cents" )
	PORT_DIPSETTING(    0x20, "10 Cents" )
	PORT_DIPSETTING(    0x18, "5 Cents" )
	PORT_DIPSETTING(    0x10, "5 Cents" )
	PORT_DIPSETTING(    0x08, "5 Cents" )
	PORT_DIPSETTING(    0x00, "5 Cents" )
	PORT_DIPNAME( 0xc0, 0xc0, "Progressive Sign" )     PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "Single Level" )
	PORT_DIPSETTING(    0x40, "Multi Level" )   // use this setting *only* if using external sign.
	PORT_DIPSETTING(    0x00, "Test Mode" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Button Lockout" )       PORT_DIPLOCATION("DSW4:1")  // this is used to lockout the button panel switches...
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Lockout Polarity" )     PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPNAME( 0x0c, 0x0c, "Printer Type" )         PORT_DIPLOCATION("DSW4:3,4")
	PORT_DIPSETTING(    0x0c, "Epson 267A" )
	PORT_DIPSETTING(    0x08, "Star 300" )
	PORT_DIPSETTING(    0x04, "Ithaca 76" )
	PORT_DIPSETTING(    0x00, "Citizen 3541" )
	PORT_DIPNAME( 0x10, 0x10, "Paper Low Sensor" )      PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Not Used (Leave OFF)" )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Not Used (Leave OFF)" )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Not Used (Leave OFF)" )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*****************************************************
*            Graphics Layouts & Decode               *
*****************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_chry10 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_goldfrui =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_cb3e =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 2*8+0, 2*8+1, 3*8+0, 3*8+1, 0*8+0, 0*8+1, 1*8+0, 1*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tilelayoutbl =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout tilelayout_chry10 =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout tilelayout_cb3e =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 2*8+0, 2*8+1,3*8+0, 3*8+1, 0, 1, 1*8+0, 1*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tiles8x8x3_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x32x4_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};

// cmasterc set
static const gfx_layout tiles8x32x4alt_layout =
{
	8,32,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 11, 10, 9, 8 },
	{ 0*16, 1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16,17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16,25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	16*32
};

static const uint32_t layout_xoffset[128] =
{
	STEP32(0*128,4),STEP32(1*128,4),STEP32(2*128,4),STEP32(3*128,4)
};

static const uint32_t layout_yoffset[128] =
{
	STEP32(0*16384, 512),STEP32(1*16384,512),STEP32(2*16384,512),STEP32(3*16384,512)
};

static const gfx_layout tiles128x128x4_layout =
{
	128,128,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	4 * 16384, /* object takes 8 consecutive bytes */
	layout_xoffset,
	layout_yoffset
};


static const uint32_t layout_xoffset256[256] =
{
	STEP32(0*128,4),STEP32(1*128,4),STEP32(2*128,4),STEP32(3*128,4), STEP32(4*128,4), STEP32(5*128,4), STEP32(6*128,4), STEP32(7*128,4)
};

static const uint32_t layout_yoffset256[256] =
{
	STEP32(0*32768, 1024),STEP32(1*32768,1024),STEP32(2*32768,1024),STEP32(3*32768,1024),STEP32(4*32768,1024), STEP32(5*32768,1024),STEP32(6*32768,1024),STEP32(7*32768,1024)
};


static const gfx_layout tiles256x128x4_layout =
{
	256,256,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	8 * 32768, /* object takes 8 consecutive bytes */
	layout_xoffset256,
	layout_yoffset256
};

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static const gfx_layout tiles8x32x5_layout =
{
	8,32,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};
#endif

static const gfx_layout cb3c_tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout cb3c_tiles8x32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32,
		13*32, 14*32, 15*32, 16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
		24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};

static const gfx_layout tiles8x8x4_layout =
{
	8, 8,
	RGN_FRAC(1,4),  /* 4096 tiles */
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout sangho_charlayout =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,1),  /* 4096 characters */
	4,              /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout sangho_tilelayout =
{
	8,32,           /* 8*32 characters */
	RGN_FRAC(1,1),  /* 1024 tiles */
	4,              /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout super9_charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 4*32, 2*32, 6*32, 1*32, 5*32, 3*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout super9_tilelayout =  // Green is OK. Red needs normal goldstar order...
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 1, 0, 1*8+1, 1*8+0, 2*8+1, 2*8+0, 3*8+1, 3*8+0 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout flaming7_charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 2*8+0, 2*8+1, 3*8+0, 3*8+1, 0*8+0, 0*8+1, 1*8+0, 1*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout flaming7_tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 2*8+0, 2*8+1, 3*8+0, 3*8+1, 0, 1, 1*8+0, 1*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
		32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
		64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
		96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout flam7_tw_charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout flam7_tw_tilelayout =  // FIXME
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 1*8+0, 1*8+1, 2*8+1, 0, 1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
		32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
		64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
		96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static GFXDECODE_START( gfx_goldstar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_bl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_ml )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_goldfrui )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_goldfrui,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_chry10 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_chry10,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_chry10, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_cb3c )
	GFXDECODE_ENTRY( "gfx1", 0, cb3c_tiles8x8_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, cb3c_tiles8x32_layout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_cb3e )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_cb3e,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_cb3e, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_ncb3 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_bingownga )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x6000, tiles8x32x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_magodds )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_cm )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
GFXDECODE_END

static GFXDECODE_START( gfx_cmbitmap )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
	GFXDECODE_ENTRY( "user1", 0, tiles128x128x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_cmasterc )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32x4alt_layout, 128+64, 4 )
	GFXDECODE_ENTRY( "user1", 0, tiles128x128x4_layout, 128, 4 )
GFXDECODE_END


static GFXDECODE_START( gfx_cmast91 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
	GFXDECODE_ENTRY( "user1", 0, tiles256x128x4_layout, 128, 4 ) // wrong... FIXME.
GFXDECODE_END

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static GFXDECODE_START( gfx_cmasterb )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x5_layout, 0, 4 )
GFXDECODE_END
#endif

static GFXDECODE_START( gfx_megaline )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_sangho )
	GFXDECODE_ENTRY( "gfx1", 0, sangho_charlayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sangho_tilelayout, 128, 8 )
/* 7*16,16 title girl in 1st color
   6*16,16 watermelon in game
   4*16,16 blueberry in game
   3*16,16 cherries in game
   2*16,16 oranges and title girl in game
   1*16,16 nines in game
*/
GFXDECODE_END

static GFXDECODE_START( gfx_super9 )
	GFXDECODE_ENTRY( "gfx1", 0, super9_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, super9_tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_flaming7 )  // gfx 2 still wrong...
	GFXDECODE_ENTRY( "gfx1", 0, flaming7_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, flaming7_tilelayout, 104,  8 )
GFXDECODE_END

static GFXDECODE_START( gfx_flam7_tw )  // gfx 2 still wrong...
	GFXDECODE_ENTRY( "gfx1", 0, flam7_tw_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, flam7_tw_tilelayout, 104,  8 )
GFXDECODE_END


static const gfx_layout tiles8x32_4bpp_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8,
		13*8, 14*8, 15*8, 16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	32*8
};

static const gfx_layout tiles8x8_3bpp_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_nfm )
	GFXDECODE_ENTRY( "tilegfx", 0, tiles8x8_3bpp_layout, 0, 16 )
	GFXDECODE_ENTRY( "reelgfx", 0, tiles8x32_4bpp_layout, 0, 16 )
GFXDECODE_END


static const gfx_layout tiles8x8x3_miss1bpp_layout =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 1, 2, 3 },
	{ 8, 12, 0, 4, 24, 28, 16, 20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout tiles8x8x4alt_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout tiles8x32x4alt2_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ STEP32(0,32) },
	32*32
};


static GFXDECODE_START( gfx_unkch )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4alt_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4alt2_layout, 0, 16 )
GFXDECODE_END

static const gfx_layout tilescherrys_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 3, 2, 1, 0 },
	{  8, 12, 0, 4, 24, 28, 16, 20 },
	{ STEP32(0,32) },
	32*32
};

static GFXDECODE_START( gfx_cherrys )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_miss1bpp_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilescherrys_layout, 128,  8 )
GFXDECODE_END


static const gfx_layout tiles8x32x4pkr_layout =
{
	8,32,           /* 8*32 characters */
	RGN_FRAC(1,1),  /* 1024 characters */
	4,              /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
		24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	8*32*4          /* every char takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_pkrmast )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4pkr_layout, 128+64, 16 )
GFXDECODE_END


static const gfx_layout cm97_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 8, 12, 0, 4, 24, 28, 16, 20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout cm97_layout32 =
	{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32,
		12*32, 13*32, 14*32, 15*32, 16*32, 17*32, 18*32, 19*32, 20*32, 21*32,
		22*32, 23*32, 24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};

static GFXDECODE_START( gfx_cm97 )
	GFXDECODE_ENTRY( "gfx", 0, cm97_layout,   0x0, 32 )
	GFXDECODE_ENTRY( "gfx", 0, cm97_layout32, 0x0, 32 )
GFXDECODE_END


WRITE8_MEMBER(wingco_state::system_outputa_w)
{
	//popmessage("system_outputa_w %02x",data);
}


WRITE8_MEMBER(wingco_state::system_outputb_w)
{
	//popmessage("system_outputb_w %02x",data);
}


WRITE8_MEMBER(wingco_state::system_outputc_w)
{
	m_nmi_enable = data & 8;
	m_vidreg = data & 2;
	//popmessage("system_outputc_w %02x",data);

	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE8_MEMBER(goldstar_state::ay8910_outputa_w)
{
	//popmessage("ay8910_outputa_w %02x",data);
}

WRITE8_MEMBER(goldstar_state::ay8910_outputb_w)
{
	//popmessage("ay8910_outputb_w %02x",data);
}


MACHINE_CONFIG_START(goldstar_state::goldstar)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(goldstar_map)
	MCFG_DEVICE_IO_MAP(goldstar_readport)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goldstar);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233, 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	MCFG_DEVICE_ADD("oki", OKIM6295, OKI_CLOCK, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


MACHINE_CONFIG_START(goldstar_state::goldstbl)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(goldstar_map)
	MCFG_DEVICE_IO_MAP(goldstar_readport)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bl);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233, 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	MCFG_DEVICE_ADD("oki", OKIM6295, OKI_CLOCK, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

void goldstar_state::moonlght(machine_config &config)
{
	goldstbl(config);
	m_gfxdecode->set_info(gfx_ml);
}

void goldstar_state::goldfrui(machine_config &config)
{
	goldstbl(config);
	m_gfxdecode->set_info(gfx_goldfrui);
}


MACHINE_CONFIG_START(sanghopm_state::star100)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(star100_map)
	MCFG_DEVICE_IO_MAP(star100_readport)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sanghopm_state, screen_update_sangho)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	PALETTE(config, m_palette).set_entries(0x100);
	RAMDAC(config, "ramdac", 0, "palette").set_addrmap(0, &sanghopm_state::ramdac_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sangho);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(sanghopm_state, sangho)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW5");
	aysnd.port_b_read_callback().set_ioport("DSW6");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	MCFG_DEVICE_ADD("oki", OKIM6295, OKI_CLOCK, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


MACHINE_CONFIG_START(goldstar_state::super9)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(goldstar_map)
//  MCFG_DEVICE_PROGRAM_MAP(nfm_map)
	MCFG_DEVICE_IO_MAP(goldstar_readport)
//  MCFG_DEVICE_IO_MAP(unkch_portmap)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_super9);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233, 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	MCFG_DEVICE_ADD("oki", OKIM6295, OKI_CLOCK, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


void goldstar_state::cm_palette(palette_device &palette) const
{
	// BBGGGRRR
	uint8_t const *const proms = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const data = proms[0x000 + i] | (proms[0x100 + i] << 4);
		palette.set_pen_color(i, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}

void goldstar_state::cmast91_palette(palette_device &palette) const
{
	uint8_t const *const proms = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		int const b = pal4bit(proms[0x000 + i]);
		int const g = pal4bit(proms[0x100 + i]);
		int const r = pal4bit(proms[0x200 + i]);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void goldstar_state::lucky8_palette(palette_device &palette) const
{
	// BBGGGRRR
	uint8_t const *proms;

	proms = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const data = proms[0x000 + i] | (proms[0x100 + i] << 4);
		palette.set_pen_color(i, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}

	proms = memregion("proms2")->base();
	for (int i = 0; i < 0x20; i++)
	{
		uint8_t const data = proms[i];
		palette.set_pen_color(i + 0x80, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


MACHINE_CONFIG_START(cb3_state::ncb3)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(ncb3_map)
	MCFG_DEVICE_IO_MAP(ncb3_readwriteport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN3");   //Player2 controls, confirmed

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN1");
	m_ppi[1]->in_pb_callback().set_ioport("IN2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END

void cb3_state::cb3c(machine_config &config)
{
	ncb3(config);
	m_gfxdecode->set_info(gfx_cb3c);
}

void cb3_state::cb3e(machine_config &config)
{
	ncb3(config);
	m_gfxdecode->set_info(gfx_cb3e);
}

void cb3_state::chrygld(machine_config &config)
{
	ncb3(config);
	m_gfxdecode->set_info(gfx_chry10);
}

void cb3_state::cherrys(machine_config &config)
{
	ncb3(config);
	m_gfxdecode->set_info(gfx_cherrys);
}

void cb3_state::cm97(machine_config &config)
{
	ncb3(config);
	m_gfxdecode->set_info(gfx_cm97);
}


MACHINE_CONFIG_START(goldstar_state::wcherry)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(wcherry_map)
	MCFG_DEVICE_IO_MAP(wcherry_readwriteport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN3");   //Player2 controls, confirmed

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN1");
	m_ppi[1]->in_pb_callback().set_ioport("IN2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cb3e);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(cmaster_state::cm)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(cm_map)
	MCFG_DEVICE_IO_MAP(cm_portmap)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("DSW1");
	m_ppi[1]->in_pb_callback().set_ioport("DSW2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW3");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cmbitmap);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, cherrym)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END

void cmaster_state::cmasterc(machine_config &config)
{
	cm(config);
	m_gfxdecode->set_info(gfx_cmasterc);
}


MACHINE_CONFIG_START(goldstar_state::cmast91)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(cm_map)
	MCFG_DEVICE_IO_MAP(cmast91_portmap)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("DSW1");
	m_ppi[1]->in_pb_callback().set_ioport("DSW2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW3");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_cmast91)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cmast91);
	PALETTE(config, m_palette, FUNC(goldstar_state::cmast91_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, cherrym)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END



WRITE_LINE_MEMBER(wingco_state::masked_irq)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

MACHINE_CONFIG_START(wingco_state::lucky8)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(lucky8_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");
	m_ppi[2]->out_pa_callback().set(FUNC(wingco_state::system_outputa_w));
	m_ppi[2]->out_pb_callback().set(FUNC(wingco_state::system_outputb_w));
	m_ppi[2]->out_pc_callback().set(FUNC(wingco_state::system_outputc_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, wingco_state, masked_irq))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette)).set_format(palette_device::BGR_233, 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ym2149_device &aysnd(YM2149(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set(FUNC(goldstar_state::ay8910_outputa_w));
	aysnd.port_b_write_callback().set(FUNC(goldstar_state::ay8910_outputb_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(wingco_state::bingowng)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(lucky8_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");
	m_ppi[2]->out_pa_callback().set(FUNC(wingco_state::system_outputa_w));
	m_ppi[2]->out_pb_callback().set(FUNC(wingco_state::system_outputb_w));
	m_ppi[2]->out_pc_callback().set(FUNC(wingco_state::system_outputc_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wingco_state, screen_update_bingowng)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, wingco_state, masked_irq))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(wingco_state, bingowng)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set(FUNC(goldstar_state::ay8910_outputa_w));
	aysnd.port_b_write_callback().set(FUNC(goldstar_state::ay8910_outputb_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END

void wingco_state::bingownga(machine_config &config)
{
	bingowng(config);
	m_gfxdecode->set_info(gfx_bingownga);
}


MACHINE_CONFIG_START(wingco_state::flam7_w4)
	lucky8(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(flaming7_map)

	m_ppi[0]->out_pc_callback().set(FUNC(wingco_state::fl7w4_outc802_w));

	MCFG_DEVICE_ADD(m_fl7w4_id, DS2401)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(wingco_state::flaming7)
	lucky8(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(flaming7_map)

	m_gfxdecode->set_info(gfx_flaming7);

	// to do serial protection.
	m_ppi[0]->out_pc_callback().set(FUNC(wingco_state::fl7w4_outc802_w));

	MCFG_DEVICE_ADD(m_fl7w4_id, DS2401)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(wingco_state::flam7_tw)
	lucky8(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(flaming7_map)

	m_gfxdecode->set_info(gfx_flam7_tw);

	// to do serial protection.
	m_ppi[0]->out_pc_callback().set(FUNC(wingco_state::fl7w4_outc802_w));

	MCFG_DEVICE_ADD(m_fl7w4_id, DS2401)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(wingco_state::mbstar)
	lucky8(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(mbstar_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(wingco_state, screen_update_mbstar)
MACHINE_CONFIG_END



void wingco_state::magodds_palette(palette_device &palette) const
{
	uint8_t const *const proms = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const b = pal4bit(proms[0x000 + i]);
		uint8_t const g = pal4bit(proms[0x100 + i]);
		uint8_t const r = pal4bit(proms[0x200 + i]);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

MACHINE_CONFIG_START(wingco_state::magodds)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(magodds_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");
	m_ppi[2]->out_pa_callback().set(FUNC(wingco_state::system_outputa_w));
	m_ppi[2]->out_pb_callback().set(FUNC(wingco_state::system_outputb_w));
	m_ppi[2]->out_pc_callback().set(FUNC(wingco_state::system_outputc_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wingco_state, screen_update_magical)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, wingco_state, masked_irq))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_magodds);
	PALETTE(config, m_palette, FUNC(wingco_state::magodds_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(wingco_state, magical)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.00)  // shut up annoying whine

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set(FUNC(goldstar_state::ay8910_outputa_w));
	aysnd.port_b_write_callback().set(FUNC(goldstar_state::ay8910_outputb_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.80);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(goldstar_state::kkotnoli)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(kkotnoli_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW1");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

MACHINE_CONFIG_END


MACHINE_CONFIG_START(goldstar_state::ladylinr)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(ladylinr_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("DSW1");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	AY8930(config, "aysnd", AY_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.50); // unused?
MACHINE_CONFIG_END


MACHINE_CONFIG_START(wingco_state::wcat3)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(wcat3_map)
	//MCFG_DEVICE_IO_MAP(goldstar_readport)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW2");
	m_ppi[2]->out_pa_callback().set(FUNC(wingco_state::system_outputa_w));
	m_ppi[2]->out_pb_callback().set(FUNC(wingco_state::system_outputb_w));
	m_ppi[2]->out_pc_callback().set(FUNC(wingco_state::system_outputc_w));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("snsnd", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set(FUNC(goldstar_state::ay8910_outputa_w));
	aysnd.port_b_write_callback().set(FUNC(goldstar_state::ay8910_outputb_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

MACHINE_CONFIG_END


/* diff with cm machine driver: gfxdecode, OKI & portmap */
MACHINE_CONFIG_START(cmaster_state::amcoe1)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(cm_map)
	MCFG_DEVICE_IO_MAP(amcoe1_portmap)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("DSW1");
	m_ppi[1]->in_pb_callback().set_ioport("DSW2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW3");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cm);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, cherrym)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	MCFG_DEVICE_ADD("oki", OKIM6295, OKI_CLOCK, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/* diff with cm machine driver: gfxdecode, OKI, portmap & tilemaps rect size/position */
MACHINE_CONFIG_START(cmaster_state::amcoe1a)
	amcoe1(config);

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(cmaster_state, screen_update_amcoe1a)
MACHINE_CONFIG_END


/* diff with cm machine driver: gfxdecode, AY8910 volume & portmap */
MACHINE_CONFIG_START(cmaster_state::amcoe2)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(cm_map)
	MCFG_DEVICE_IO_MAP(amcoe2_portmap)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("DSW1");
	m_ppi[1]->in_pb_callback().set_ioport("DSW2");
	m_ppi[1]->in_pc_callback().set_ioport("DSW3");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cm);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, cherrym)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 2.00); /* analyzed for clips */
MACHINE_CONFIG_END

MACHINE_CONFIG_START(cmaster_state::nfm)
	amcoe2(config);

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(nfm_map)

	m_gfxdecode->set_info(gfx_nfm);
MACHINE_CONFIG_END


WRITE_LINE_MEMBER(unkch_state::vblank_irq)
{
	if (state && m_vblank_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

MACHINE_CONFIG_START(unkch_state::unkch)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(unkch_map)
	MCFG_DEVICE_IO_MAP(unkch_portmap)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(unkch_state, screen_update_unkch)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, unkch_state, vblank_irq))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unkch);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 512);

	MCFG_VIDEO_START_OVERRIDE(unkch_state, unkch)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	/* payout hardware */
	TICKET_DISPENSER(config, m_ticket_dispenser, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW);
MACHINE_CONFIG_END



// hw unknown - should be somewhat similar to cm
MACHINE_CONFIG_START(goldstar_state::pkrmast)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(cm_map)
	MCFG_DEVICE_IO_MAP(pkrmast_portmap)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(HOLDLINE("maincpu", 0))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pkrmast);
	PALETTE(config, m_palette, FUNC(goldstar_state::cm_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, cherrym)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(unkch_state::megaline)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(megaline_map)
	MCFG_DEVICE_IO_MAP(megaline_portmap)

	//MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	//MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	//MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_megaline);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);
//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state,goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("sn1", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn2", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn3", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set(FUNC(goldstar_state::ay8910_outputa_w));
	aysnd.port_b_write_callback().set(FUNC(goldstar_state::ay8910_outputb_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);

MACHINE_CONFIG_END


MACHINE_CONFIG_START(unkch_state::bonusch)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 12_MHz_XTAL / 2)
	MCFG_DEVICE_PROGRAM_MAP(bonusch_map)
	MCFG_DEVICE_IO_MAP(bonusch_portmap)

	MCFG_DEVICE_ADD("mcu", I80C51, 12_MHz_XTAL)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goldstar_state, screen_update_goldstar)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_megaline);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette), 256);

	MCFG_VIDEO_START_OVERRIDE(goldstar_state, goldstar)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("sn1", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn2", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn3", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn4", SN76489, PSG_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

MACHINE_CONFIG_END

MACHINE_CONFIG_START(unkch_state::feverch)

	MCFG_DEVICE_ADD("maincpu", Z80, 12'000'000 / 2) // clock not verified
	MCFG_DEVICE_PROGRAM_MAP(feverch_map)
	MCFG_DEVICE_IO_MAP(feverch_portmap)

	I8255A(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");
	m_ppi[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set_ioport("IN3");
	m_ppi[1]->in_pb_callback().set_ioport("IN4");
	m_ppi[1]->in_pc_callback().set_ioport("DSW1");

	I8255A(config, m_ppi[2]);
	m_ppi[2]->in_pa_callback().set_ioport("DSW4");
	m_ppi[2]->in_pb_callback().set_ioport("DSW2");
	m_ppi[2]->in_pc_callback().set_ioport("DSW3");

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(unkch_state, screen_update_unkch)
	MCFG_SCREEN_PALETTE(m_palette)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ncb3);
	PALETTE(config, m_palette, FUNC(goldstar_state::lucky8_palette)).set_format(palette_device::BGR_233, 256);

	MCFG_VIDEO_START_OVERRIDE(unkch_state, unkch)

	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("sn1", SN76489A, 12'000'000 / 12) // actually SN76489AN, clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn2", SN76489A, 12'000'000 / 12) // actually SN76489AN, clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DEVICE_ADD("sn3", SN76489A, 12'000'000 / 12) // actually SN76489AN, clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*
  Golden Star (bootleg of Cherry 1 Gold)
  Processor: ZILOG Z0840006PSC Z80 CPU 9745 RN

  EPROM:

    Name:  1
    File:  gs1.bin
    Type:  TMS JL 27C010A-12 LX78AC96P

    Name:  2
    File:  gs2.bin
    Type:  TMS JL 27C010A-10 LX73A5J4P

    Nome:  3
    File:  gs3.bin
    Type:  TMS JL 29C010A-12 LX78AC96P

    Nome:  4
    File:  gs4.bin
    Type:  TMS JL 27C010A-10 LX73A5J4P

  Other IC's on MB

  - ACTEL A1020B PL84C 9713
  - 9703R MS6264L-70PC
  - HOLTEK HT 6116-70 9806K0625

*/
ROM_START( goldstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/*
  Golden Star (blue version)

  This is the blue PCB version. EPROMs are marked with colored stickers.

  The red and yellow EPROMs are for graphics. The red is identical to the one found in normal golden star,
  while yellow match only 90% (probably the logo)

  The blue one match the golden star ROM 1 and contains ADCPM audio.

*/
ROM_START( goldstbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


/*
  Star 100, by Sang Ho.

  PCB SANGHO PM-003 (VER-B2).
  Different hardware, but seems to be close to GoldStar.
  Also different memory map.

  27C020 socket for OKI samples is unpopulated.

*/
ROM_START( star100 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "str-100_051212__27c512.1e",  0x00000, 0x10000, CRC(6c73ae4e) SHA1(8476b77a190a653b2a47682072bc9b4db594c02e) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "str_l3__flash29c011a-15.3l", 0x00000, 0x20000, CRC(89bf5935) SHA1(f8af107e21a9157ea5056eedbda36a1b99c5df5b) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "str_m3__flash29c011a-15.3m", 0x00000, 0x20000, CRC(fff9ea0e) SHA1(6125c99e684ac639a0f85cbb00c26131a23324aa) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "27c020.bin",  0x0000, 0x20000, NO_DUMP )
ROM_END


/*
  Crazy Bonus, by Sang Ho.

  PCB SANGHO PM-001 SW-008.
  Close to star100 hardware, but with 5x DSW banks.

  27C020 socket place for OKI samples is blind.

  This dump is not from the original Sang Ho board, it's from a Poker Master stealth conversion kit
  Plug-in daughterboard dated 1997
  This set displays Crazy Co. copyright

*/
ROM_START( crazybon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crazy_14_030418.bin",  0x00000, 0x10000, CRC(0071fb2a) SHA1(771b9b2b9fdf11dafc5ec0dbababc181d2ce4c75) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "crazy_h3.bin", 0x00000, 0x20000, CRC(6b3692b5) SHA1(ffdcd4e59d7c009fd76a65e8f87642da35f996f4) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "crazy_j3.bin", 0x00000, 0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END

ROM_START( crazybona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_main.bin",  0x00000, 0x10000, CRC(61a5585d) SHA1(5fdcf80f62ed21c9e903da64cf67ed39ec55b8b9) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "crazy_h3.bin", 0x00000, 0x20000, CRC(6b3692b5) SHA1(ffdcd4e59d7c009fd76a65e8f87642da35f996f4) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "crazy_j3.bin", 0x00000, 0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END

/*
Cherry Gold  (Cherry 10)

Anno    1997
Produttore
N.revisione W4BON (rev.1)

CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46)
1x D71055C (u40)
1x WF19054 (u39)(equivalent to AY-3-8910)
1x SN76489AN (u38)
1x PIC16F84 (on a small daughterboard)(read protected)
1x oscillator 12.000

ROMs

1x TMS27C256 (u3)
1x TMS27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
2x PALCE20V8H (pl1,pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x GAL22V10B (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/
ROM_START( chry10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ver.1h2.u20",       0x0000, 0x10000, CRC(85bbde06) SHA1(f44d335feb4697b195e9fc7e5aeaabf099e21ed8) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic16f84.bad.dump", 0x00000, 0x014f4, BAD_DUMP CRC(876ff1ed) SHA1(fcd6892e2b8371030af15e4d8c9f4a351ce0551c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c010.u1",         0x00000, 0x20000, CRC(05515cf8) SHA1(366dd44ae93bdc4cf456f97f38edac83441cbc89) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",              0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u2",         0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02e5, "palgal", 0 )
	ROM_LOAD( "palce20v8h.pl1",    0x00000, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h.pl2",    0x00000, 0x0157, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl4",    0x00000, 0x0117, NO_DUMP )
	ROM_LOAD( "gal22v10b.pl5",     0x00000, 0x02e5, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl6",    0x00000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl7",    0x00000, 0x0117, NO_DUMP )
ROM_END


/*
Cherry I Gold

Anno    1998
Produttore
N.revisione W4BON (rev.1)


CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46) (missing)
1x D71055C (u40) (missing)
1x YM2149 (u39)
1x SN76489AN (u38)
1x oscillator 12.0C45

ROMs

1x I27256 (u3)
1x I27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
1x GAL20V8 (pl1)(read protected)
1x PALCE20V8H (pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x PEEL22CV10 (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/
ROM_START( chrygld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ol-v9.u20",       0x00000, 0x10000, CRC(b61c0695) SHA1(63c44b20fd7f76bdb33331273d2610e8cfd31add) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "ol-la.u1",        0x00000, 0x20000, CRC(c3c912f1) SHA1(a2131f092ae1971f79a11d6a18b031cd98529320) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",            0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u2",       0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02dd, "palgal", 0 )
	ROM_LOAD( "gal20v8.pl1",     0x00000, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h.pl2",  0x00000, 0x0157, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl4",  0x00000, 0x0117, NO_DUMP )
	ROM_LOAD( "peel22cv10a.pl5", 0x00000, 0x02dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl6",  0x00000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce16v8h.pl7",  0x00000, 0x0117, NO_DUMP )
ROM_END


/* Moon Light (V.0629)
   Rip off / clone of Gold Star.

   The program ROM is double size and stores two different programs.
   Whilst we have not idea about the real addressing, we can support
   both sets separately.

*/
ROM_START( moonlght )
	ROM_REGION( 0x20000, "maincpu", 0 )  // using only the first half of the program ROM.
	ROM_LOAD( "4.bin",       0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )    // low program, normal gfx

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

ROM_START( moonlghta )
	ROM_REGION( 0x20000, "maincpu", 0 )  // using only the second half of the program ROM.
	ROM_LOAD( "4.bin",       0x10000, 0x10000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )   // high program, normal gfx
	ROM_CONTINUE(            0x00000, 0x10000)

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/* Moon Light (V.02L0A)
   Rip off / clone of Gold Star.

   The program ROM is double size and stores two different programs.
   Whilst we have not idea about the real addressing, we can support
   both sets separately.

   GFX devices are 4 times bigger and contains 4 times the same data.
   Maybe the manufacturers run out of proper devices...

   The background is not set properly due to a palette error. The program ROM stores
   the palette at offset 0xC700 onwards... The value stored at 0xC780 (color 0x80) should
   be black to mask the reels tilemaps and turn them 'invisible'. This program has a value
   of 0x40 instead, turning the tilemaps blue and therefore visible. The results is an odd
   effect that shouldn't be there. Maybe is product of a bad dump. Need to be checked with
   the real board.

   The hardware currently uses only the second half of the program ROM (high program), that
   replaces the double-up's cards graphics with 'drakkars' (scandinavian / viking ships).

----------------------------------------------------------------------------------------
28.bin                                          FIXED BITS (00xxxxxx)
29.bin                                                 00xxxxxxxxxxxxxxx = 0xFF
                        moon-gfx1.bin           BADADDR     --xxxxxxxxxxxxxxxxx
                        moon-gfx2.bin           FIXED BITS (00xxxxxx)
                        moon-gfx2.bin           BADADDR     --xxxxxxxxxxxxxxxxx
29.bin                  moon-gfx1.bin [1/4]      IDENTICAL
29.bin                  moon-gfx1.bin [2/4]      IDENTICAL
29.bin                  moon-gfx1.bin [3/4]      IDENTICAL
29.bin                  moon-gfx1.bin [4/4]      IDENTICAL
4.bin        [1/4]      moon-main.bin [1/4]      99.615479%
4.bin        [3/4]      moon-main.bin [3/4]      99.426270%
4.bin        [2/4]      moon-main.bin [2/4]      97.201538%
4.bin        [4/4]      moon-main.bin [4/4]      95.953369%
28.bin                  moon-gfx2.bin [1/4]      94.188690%
28.bin                  moon-gfx2.bin [2/4]      94.188690%
28.bin                  moon-gfx2.bin [3/4]      94.188690%
28.bin                  moon-gfx2.bin [4/4]      94.188690%
*/
ROM_START( moonlghtb )
	ROM_REGION( 0x20000, "maincpu", 0 )  // using only the first half of the program ROM.
	ROM_LOAD( "moon-main.bin",       0x00000, 0x20000, CRC(0a4b5dd0) SHA1(825801e9b72c10fed8e07f42b3b475688bdbd878) )  // low program, normal gfx

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )    // Normal GFX, from the other PCB

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )    // Normal GFX, from the other PCB

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "moon-sound.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

ROM_START( moonlghtc )
	ROM_REGION( 0x20000, "maincpu", 0 )  // using only the second half of the program ROM.
	ROM_LOAD( "moon-main.bin",       0x10000, 0x10000, CRC(0a4b5dd0) SHA1(825801e9b72c10fed8e07f42b3b475688bdbd878) )  // high program, alt gfx
	ROM_CONTINUE(                    0x00000, 0x10000)

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "moon-gfx2.bin",      0x00000, 0x80000, CRC(2ce5b722) SHA1(feb87fbf3b8d875842df80cd1edfef5071ed60c7) )  // Alt GFX set. Ships instead of cards

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "moon-gfx1.bin",      0x00000, 0x80000, CRC(ea7d4234) SHA1(4016227aabf176c6e0fd822ebc59cade811f4ce8) )  // Alt GFX set. Ships instead of cards

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "moon-sound.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


/* Gold Fruit

   Graphics are packed/encoded in a different way.
   Game rate is fixed in 40%.
   Coin A and B are fixed to 100 credits by pulse.

*/
ROM_START( goldfrui )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c1000.u6",  0x0000, 0x10000, CRC(84b982fc) SHA1(39f401da52a9df799f3fe6bbeb7cad493911b831) )
	ROM_CONTINUE( 0x0000, 0x10000) /* Discarding 1st half 0xff filled */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c1000.u24",      0x00000, 0x20000, CRC(9642c9c2) SHA1(10fdced265ef4a9a5494d8df0432337df4ecec7f) ) //FIXED BITS (00xxxxxx)

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "27c1000.u25",      0x00000, 0x08000, CRC(5ce73db6) SHA1(e93948f6a44831583e0779da3158d7b5e33bcca7) )
	ROM_CONTINUE( 0x0000, 0x08000) /* Discarding 1st quarter 0xff filled */
	ROM_CONTINUE( 0x0000, 0x08000) /* Discarding 2nd quarter 0xff filled */
	ROM_CONTINUE( 0x0000, 0x08000) /* Discarding 3rd quarter 0xff filled */

	ROM_REGION( 0x40000, "oki", 0 ) // Audio ADPCM
	ROM_LOAD( "27c1000.u57",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


/*
    Super Nove by Playmark

    bp 2db
    the next call ($0C33) hangs the game
    since there are ascii strings there
    instead of code.

*/
ROM_START( super9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27e010.30",       0x0000, 0x10000, CRC(1aaea8d3) SHA1(71395a6d74a7cd55606daa57d17ff4628aa5f577) )
	ROM_IGNORE(                          0x10000)   /* Discarding 2nd half */
//  ROM_LOAD( "27e010.30",       0x0000, 0x10000, CRC(1aaea8d3) SHA1(71395a6d74a7cd55606daa57d17ff4628aa5f577) )
//  ROM_CONTINUE(                0x0000, 0x10000)   /* Discarding 1st half */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "nearcpu.bin",      0x00000, 0x20000, CRC(643cff6f) SHA1(305ca9182c3f6d69e09be38b854b3d7bdfa75439) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "27e010.29",      0x00000, 0x08000, CRC(5ea46322) SHA1(147078689f0194affcdcf0e8f8e17fe8a113a377) )
	ROM_CONTINUE( 0x0000, 0x08000) // Discarding 1nd quarter 0xff filled
	ROM_CONTINUE( 0x0000, 0x08000) // Discarding 2nd quarter 0xff filled
	ROM_CONTINUE( 0x0000, 0x08000) // Discarding 3nd quarter 0xff filled

	ROM_REGION( 0x40000, "oki", 0 ) /* Audio ADPCM */
	ROM_LOAD( "27c1001.27",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( ncb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.512", 0x00000, 0x10000, CRC(1f669cd0) SHA1(fd394119e33c017507fde87a710577e37dcdec07) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	//ROM_LOAD( "4.256", 0x10000, 0x08000, BAD_DUMP CRC(a390f1f2) SHA1(0a04a5af51f91f04773125f703c7cd3397d192f2) ) // FIXED BITS (xxxx1xxx) - use main_7.256 from set below instead?
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )    /* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


/*

mame -romident cb3.zip
cpu_u6.512          NO MATCH
main_3.764          = 5.764                 New Cherry Bonus 3
main_4.764          = 1.764                 New Cherry Bonus 3
main_5.256          = 2.256                 New Cherry Bonus 3
main_6.256          = 3.256                 New Cherry Bonus 3
main_7.256          NO MATCH

*/
ROM_START( cb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu_u6.512", 0x00000, 0x10000, CRC(d17c936b) SHA1(bf90edd214118116da675bcfca41247d5891ac90) ) // encrypted??

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "main_5.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "main_6.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	/* 2 roms missing - the first 2 roms below taken from above set */
	ROM_LOAD( "7.764",      0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764",      0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "main_3.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "main_4.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )    /* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


/*
CB3A
Known differences with ncb3:

- Double-Up rate: 50% and 80% instead of 80% and 90%.

*/
ROM_START( cb3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb3a01.bin", 0x00000, 0x10000, CRC(53b099ab) SHA1(612d86d7f011a554903400e60e2c4a0d4f24e095) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )    /* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


ROM_START( cb3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "adatabin.bin",   0x00000, 0x10000,  CRC(db583c1b) SHA1(ea733e625922d6064ee4d8ceee4acfa6c1c7337e) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "adatabin_2.bin",   0x00000, 0x10000, CRC(48fd96fb) SHA1(193ed2be51555af80a9f0478139f28963e9d0c5e) )
	ROM_LOAD( "adatabin_3.bin",   0x10000, 0x10000, CRC(010462df) SHA1(53dd3060097f964c516d1cc5be2403a9bd5ee434) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "adatabin_4.bin",   0x00000, 0x08000, CRC(3cb4642a) SHA1(8db03c0227836d988e99f6fe4719d871ea3749ca) )

	ROM_REGION( 0x0200, "proms", 0 )    /* PROM from chrygld - wasn't in this set, is it correct?, none of the other proms have the colours? */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x0200, "proms2", 0 )   /* other roms */
	ROM_LOAD( "adatabin_1.bin",      0x00000, 0x020, CRC(87dbc339) SHA1(e5c67bc29612c8ab93857639e46608a814d471f5) )
	ROM_LOAD( "adatabin_5.bin",      0x00000, 0x180, CRC(ad267b0c) SHA1(a4cfec15ae0cde7d4fb8c278e977995680779058) )
	ROM_LOAD( "adatabin_6.bin",      0x00000, 0x010, CRC(f3d9ed7a) SHA1(594fef6323530f68c7303dcdea77b44c331e5113) )
	ROM_LOAD( "adatabin_0.bin",      0x00000, 0x100, CRC(f566e5e0) SHA1(754f04521b9eb73b34fe3de07e8f3679d1034870) )
ROM_END


ROM_START( cb3c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.bin", 0x00000, 0x10000, CRC(c42533cd) SHA1(d55b54b31c910d97418f400fc1ba78460c7183a9) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "cbt1grk.bin", 0x00000, 0x20000, BAD_DUMP CRC(c6fdebc7) SHA1(736bbe5ae7b148e529f7cb80e9ae8903203c7869) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "rbt1grb.bin", 0x00000, 0x8000, CRC(ed635dd7) SHA1(350a4b10ccfddcd6f3aaf748c15d585f0b9dc09b) )

	ROM_REGION( 0x0200, "proms", 0 ) // wasn't in this set..
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


// set marked 'pignapoke'
ROM_START( cb3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.14b", 0x00000, 0x10000, CRC(4f8be0f9) SHA1(0201bf0c1dca1570d2ccc862f10a19e0e261f7ab) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.7h", 0x00000, 0x10000, CRC(50701ce1) SHA1(3af3f124e28fda2bb3ba4f2c78fcd21c725d7ebc) )
	ROM_LOAD( "6.9h", 0x10000, 0x10000, CRC(baf1c9a2) SHA1(f75a64fa64d102a0e75919c76db638f6c34ecc9e) )
	ROM_LOAD( "7.10h",0x20000, 0x10000, CRC(87c944c4) SHA1(754b410dbcef07967f7cfdc3923df20394909e49) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.1h", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "2.3h", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "3.4h", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "4.5h", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "n82s129.13g",  0x0000, 0x0100, CRC(59ac98e4) SHA1(5fc0f1a48c49c956cdb8826e20663dc57a9175e4) )   // 1st bank colors, low 4 bits.
	ROM_LOAD_NIB_HIGH( "n82s129.14g",  0x0000, 0x0100, CRC(0d8f35bd) SHA1(0c2a0145cdaaf9beabdce241731a36b0c65f18a2) )   // 1st bank colors, high 4 bits.
	ROM_LOAD(          "dm74s288.13d", 0x0080, 0x0020, CRC(77a85e21) SHA1(3b41e0ab7cc55c5d78914d23e8289383f5bd5654) )   // 2nd bank colors
ROM_END


/*
  1x Z80.
  3x 8255 (2 have no mark).
  1x Ay-3-8910.
  5x 8 DIP switches.
  1x 12 MHz xtal.

  ROM 3v202 is the prg.
*/
ROM_START( cb3e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3v202.u22",  0x00000, 0x10000, CRC(f127d203) SHA1(d23b9e5972e797e7c18e9e8e2e70c01f381a4c4d) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.u6",      0x00000, 0x20000, CRC(e3be1d33) SHA1(5cc3b5d6e371e8bb414b552c68770666e3914ae4) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(919bd692) SHA1(1aeb66f1e4555b731858833445000593e613f74d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u1",      0x00000, 0x0100, CRC(d4eaa276) SHA1(b6598ee64ac3d41ca979c8667de8576cfb304451) )
	ROM_CONTINUE(               0x00000, 0x0100)    // 2nd half has the data.
ROM_END


/* Cherry Gold I (bootleg)
   It runs in CB3e similar hardware...

1x TMPZ84C00AP-6  u15               8-bit Microprocessor
3x D71055C        u30, u39, u40     Programmable Peripheral Interface
1x WF19054        u27               Programmable Sound Generator
1x SN76489AN      u28               Digital Complex Sound Generator
1x oscillator   unmarked  Y1

ROMs
1x D27256      1.u3
1x AM27512     3.u22
1x D27C010     2u6
1x N82S147AF   u1

RAMs
4x HM6116LP-4   u9, u10, u11, u12
1x D4016C-1     u23

PLDs
1x unknowun Cl-001 (QFP144)  CY144A    read protected
4x GAL20V8A-15LNC  pl1, pl4, pl5, pl6  read protected
2x PALCE20V8H-25PC/4   u2,u?           read protected
1x PALCE22V10H-25PC/4  u?              read protected

Others
1x 36x2 edge connector
1x 10x2 edge connector
1x pushbutton (TS)
2x trimmer (volume)(VR1,VR2)
5x 8x2 switches DIP(SW1-5)
1x battery 5,5V

Notes
PCB is marked "REV.3"
*/
ROM_START( chryglda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u22",   0x00000, 0x10000, CRC(059857c5) SHA1(f4becfda1e25ab347f55f35dc9f5818ef9344e2c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.u6",    0x00000, 0x20000, CRC(1359dfac) SHA1(78eb934055cda9e10d8e939c79bfa62262ecad7d) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",    0x00000, 0x08000, CRC(919bd692) SHA1(1aeb66f1e4555b731858833445000593e613f74d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147af.u1",   0x00000, 0x0100, CRC(d4eaa276) SHA1(b6598ee64ac3d41ca979c8667de8576cfb304451) )
	ROM_CONTINUE(               0x00000, 0x0100)    // 2nd half has the data.
ROM_END


ROM_START( cmv801 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg512",   0x0000, 0x10000, CRC(2f6e3fe9) SHA1(c5ffa51478a0dc2d8ff6a0f286cfb461011bb55d) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "m5.256",   0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "m6.256",   0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "m7.256",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "m3.64",     0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "m4.64",     0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "m1.64",     0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "m2.64",     0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, BAD_DUMP CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) ) // taken from other set, was missing in this dump

	ROM_REGION( 0x200, "proms", 0 ) // pal
	ROM_LOAD( "prom2.287", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "prom3.287", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 ) // something else?
	ROM_LOAD( "prom1.287", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*
Super Seven - 199? (>= 1995 since it cites an Italian law from 1995)
Bootleg /licensed version of Cherry Master (Dyna CM-1 V1.01 at 0xd000 of the program ROM).

PCB with a New Impeuropex sticker

- Main cpu is a Z80B (Z8400B) clocked @3MHz (derived by a 12MHZ quartz)
- Sound is generated by a AY-3-8910 clocked @1.5MHz (derived by the same 12MHZ quartz)
- One 27C512 for storing program code
- Two 27C010 for storing GFX data
- One 82S191AN BPROM
- One 82S137AN
- Five PLDs (two PALCE16V8 and three GAL20V8)
- One 3.6V lithium battery probably for storing high-scores
*/

ROM_START( super7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_1", 0x00000, 0x10000, CRC(ddfa6fe7) SHA1(0d86ec5029afd565e039fe84f7001b2dc77c919c) ) // first 0x8000 encrypted

	ROM_REGION( 0x20000, "gfx1", 0 ) // very similar to most cmv4 sets, but differently arranged
	ROM_LOAD( "27c010_2", 0x00000, 0x20000, CRC(9636d785) SHA1(8f851aae0b05ad909c48cf94142ab927145da464) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "27c010_3", 0x00000, 0x20000, CRC(a6db1162) SHA1(05019166526b0797e3eca8b72d90c325573b3d74) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF ) // no girls ROM

	ROM_REGION( 0x800, "proms", 0 ) // RGB generation
	ROM_LOAD( "82s191an.bin",  0x000, 0x800, CRC(ec546abe) SHA1(1bd92c0715ec1821fa977a67499dc8971deec9c7) )

	ROM_REGION( 0x400, "proms2", 0 )
	ROM_LOAD( "82s137an.bin",  0x000, 0x400, CRC(92975789) SHA1(4a85d169db5e298ee201fe7d4b9964b1df16992e) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "palce16v8.bin", 0x000, 0x114, NO_DUMP )
	ROM_LOAD( "palce16v8.bin", 0x000, 0x114, NO_DUMP )
	ROM_LOAD( "gal20v8.bin",   0x000, 0x114, NO_DUMP )
	ROM_LOAD( "gal20v8.bin",   0x000, 0x114, NO_DUMP )
	ROM_LOAD( "gal20v8.bin",   0x000, 0x114, NO_DUMP )
ROM_END

/*
2764.u10                m4.64                   IDENTICAL
27256.u11               m6.256                  IDENTICAL
2764.u14                m1.64                   IDENTICAL
2764.u15                m3.64                   IDENTICAL
27256.u16               m5.256                  IDENTICAL
27256.u4                m7.256                  IDENTICAL
82s129.u46              prom1.287               IDENTICAL
82s129.u70              prom3.287               IDENTICAL
82s129.u84              prom2.287               IDENTICAL
2764.u9                 m2.64                   IDENTICAL
27512.u53               prg512                  4.640198%
27256.u81                                       NO MATCH

PCB Layout
----------

|-----|  |------|  |---------------------------|
|     |--|      |--|  ROM.U4                   |
|                     ROM.U11  ROM.U10  ROM.U9 |
|_            DSW5(8) ROM.U16  ROM.U15  ROM.U14|
  |  WF19054  DSW4(8)                          |
 _|           DSW3(8) 6116                     |
|             DSW2(8) 6116         6116        |
|             DSW1(8)                          |
|    ?DIP40                        6116        |
|                                              |
|                     PROM.U46            12MHz|
|                                              |
|    8255                       PAL            |
|         ROM.U53                              |
|                                              |
|                                              |
|                                   PAL        |
|BATTERY  PROM.U70        PAL   PAL            |
|_        PROM.U84        6116  ROM.U81    Z80 |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
      ?DIP40 - Maybe another 8255 or HD6845 or something else?
*/

ROM_START( cmv4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256.u81",  0x0000, 0x1000, CRC(e27e98a3) SHA1(1eb03f6c770f25ff5e3c25a1f9b9294c6b3c61d9) )
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",   0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*

cmv4 vs. fruit2

12 and 12 files
82s129.u46                                      FIXED BITS (0000x0xx)
82s129.u46                                      BADADDR                xxxxxxx-
82s129.u70                                      FIXED BITS (0000xxxx)
82s129.u84                                      FIXED BITS (0000xxxx)
                        HOLD8_PR1.U46           FIXED BITS (0000x0xx)
                        HOLD8_PR1.U46           BADADDR                xxxxxxx-
                        HOLD8_PR2.U84           FIXED BITS (0000xxxx)
                        HOLD8_PR3.U79           FIXED BITS (0000xxxx)
                        TETRIS_5.U4             1ST AND 2ND HALF IDENTICAL
                        TETRIS_6.U11            1ST AND 2ND HALF IDENTICAL
                        TETRIS_7.U16            1ST AND 2ND HALF IDENTICAL
2764.u10                HOLDX8_3.U10            IDENTICAL
2764.u14                HOLDX8_2.U14            IDENTICAL
2764.u15                HOLDX8_4.U15            IDENTICAL
82s129.u46              HOLD8_PR1.U46            IDENTICAL
27512.u53               HOLDX8_8.U53            IDENTICAL
82s129.u70              HOLD8_PR3.U79            IDENTICAL
82s129.u84              HOLD8_PR2.U84            IDENTICAL
2764.u9                 HOLDX8_1.U9             IDENTICAL
27256.u81               HOLDX8.U81              25.512695%
27256.u11                                       NO MATCH
27256.u16                                       NO MATCH
27256.u4                                        NO MATCH
                        TETRIS_5.U4             NO MATCH
                        TETRIS_6.U11            NO MATCH
                        TETRIS_7.U16            NO MATCH
*/

ROM_START( cmv4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "holdx8.u81", 0x0000, 0x2000, CRC(2da4d267) SHA1(3deb6ce4703bb1cca2b58409a6fc5ba7cf601011) )
	ROM_CONTINUE(0x4000,0x2000)
	ROM_CONTINUE(0x2000,0x2000)
	ROM_CONTINUE(0x6000,0x2000)

	ROM_REGION( 0x30000, "gfx1", 0 )    /* double size... tetris gfx inside. */
	ROM_LOAD( "tetris_7.u16",  0x00000, 0x10000, CRC(a9e61fac) SHA1(a4093868b570af52f1cd816d523dbb6a750bc1af) )
	ROM_LOAD( "tetris_6.u11",  0x10000, 0x10000, CRC(f3e15b41) SHA1(6c44bcdcf7b29d201018e2c2cc0b6b0b62cbaf12) )
	ROM_LOAD( "tetris_5.u4",   0x20000, 0x10000, CRC(79a85560) SHA1(b028345ac2d01c643230c6e6c28189c11734aaf5) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "holdx8_4.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "holdx8_3.u10",   0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "holdx8_2.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "holdx8_1.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	// contains a bitmap? and an extra plane for gfx2, should it be used?
	ROM_LOAD( "holdx8_8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hold8_pr2.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "hold8_pr3.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "hold8_pr1.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmwm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "waterm.rom",  0x0000, 0x1000, CRC(93b6cb9b) SHA1(294e1e5909b304252c79a7d3f50fc175558e713b) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",   0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmfun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmvfun.rom",  0x0000, 0x1000, CRC(128f373e) SHA1(24d51ab669d568c004e2c94cac22eb8476ce2718) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",   0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/* looks like a bootleg of cmv4 */
ROM_START( cmaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin",   0x00000, 0x1000, CRC(ccb64229) SHA1(532f4b59952702a3609ff20239acbbacaf71f38f) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_IGNORE(0x8000) // 2nd half is identical

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "6.bin",   0x00000, 0x10000, CRC(a98d610f) SHA1(d14b3bc8bd6dc9fe2d3fb05ec08224b1a9d52bee) )
	ROM_LOAD( "7.bin",   0x10000, 0x10000, CRC(a0ffd2d6) SHA1(e78d20d3ab578ccc880bc19678782cb1f8d3671e) )
	ROM_LOAD( "8.bin",   0x20000, 0x10000, CRC(4f67fca7) SHA1(808e84b9b1f67f137528bb76b0e8aac3dceba20c) )

	// 4-9 all of these contain the same bitmap at 0x0000-0xe000
	// rom 9 matches the rom containing the bitmap in the cmv4 set, but also contains an extra gfx2 plane
	//  should that plane be used??
	ROM_REGION( 0x50000, "graphics", 0 )
	ROM_LOAD( "4.bin",   0x00000, 0x10000, CRC(52240e0f) SHA1(7b8375e1f91fdff2b4ccc2e81fbcf843f7ede292) )
	ROM_LOAD( "5.bin",   0x10000, 0x10000, CRC(763973c1) SHA1(b364f22041f1d678332554edb3c718cf0ad778b4) )
	ROM_LOAD( "1.bin",   0x20000, 0x10000, CRC(634fe2ad) SHA1(2284a09446c8928060270861d372a19c0c9d827a) )
	ROM_LOAD( "2.bin",   0x30000, 0x10000, CRC(a3d59f79) SHA1(588c45550cca673390a35af9617c68c853ff84ba) )
	ROM_LOAD( "9.bin",   0x40000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	// for now we don't copy the extra plane..
	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x0e000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x1e000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x2e000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x3e000, 0x06000, 0x2000 )

	// copy one of the versions of the bitmap
	ROM_REGION( 0x10000, "user1", 0 )
	ROM_COPY( "graphics", 0x40000, 0x00000, 0xe000 )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*  cmasterb & cmasterc

    Cherry Master II ver 8.41

    CPU : LH0080B Z80B CPU

    RAM : MB8128-15 x2

        MB8416-20 x2

        HM6116

    Sound : Winbond WF19054

    Crystal : 12.000 Mhz

    Dip SW :

    4x8 dip + 1 Switch (main test ???)
*/
ROM_START( cmasterb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u81.9",  0x0000,  0x1000, CRC(09e44314) SHA1(dbb7e9afc9a1dc0d4ce7b150324077f3f3579c02) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmezspin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ezspin.rom",  0x0000,  0x1000, CRC(de92b85c) SHA1(36e99b1444980a279293839c6db10f577b9e8657) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "up11.6", 0x08000,  0x8000, CRC(c1466efa) SHA1(d725fc507c77e66bde93d0c33bf469add15f39b9) ) // changed title
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmasterc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "msii841.u81",  0x3000,  0x1000, CRC(977db602) SHA1(0fd3d6781b654ac6befdc9278f84ca708d5d448c) )
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "msii841.u16", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "msii841.u11", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "msii841.u4",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	/* these gfx are in a different format to usual */
	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "msii841.u1",  0x0000,  0x4000, CRC(cf322ed2) SHA1(84df96229b7bdba0ab498e3bf9c77d7a7661f7b3) )
	ROM_LOAD( "msii841.u2",  0x4000,  0x4000, CRC(58c05653) SHA1(59454c07f4fe5b684d078cf97f2b1ee05b02f4ed) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "msii841.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*

this set is weird, apart from the MGRAISE.BIN it's a cherry master set, I'm assuming that somehow
there is extra hardware which causes it to run a different game contained in that rom?

the extra rom contains

MEGA DOUBLE POKER TM        COPYRIGHT 1991

as well as various

COPYRIGHT (C) 1988-90-92 GERALD DUHAMEL

strings spread across the rom

------------------

Blitz Video Poker
1991

Hardware for a real Video Poker Machine that was supposed to pay out $$$.

Contains:

1 X Z80 CPU
2 X 6505s?
1 GI AY-3-8910
2 x 8255

CH3      BIN        32,768  11-15-98  8:46a CH3.BIN
MAST9    BIN        65,536  11-15-98  8:47a MAST9.BIN
MAST5    BIN        32,768  11-15-98  8:48a MAST5.BIN
MAST6    BIN        32,768  11-15-98  8:50a MAST6.BIN
MAST7    BIN        32,768  11-15-98  8:50a MAST7.BIN
MAST1    BIN         8,192  11-15-98  8:52a MAST1.BIN
MAST2    BIN         8,192  11-15-98  8:53a MAST2.BIN
MAST3    BIN         8,192  11-15-98  8:54a MAST3.BIN
MAST4    BIN         8,192  11-15-98  8:55a MAST4.BIN
MGRAISE  BIN       131,072  11-15-98  8:58a MGRAISE.BIN Sound Amp with 6502

*/

ROM_START( cmasterbv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ch3.bin",  0x0000, 0x1000, CRC(9af51e47) SHA1(ac002c218502430a3e45259776ca409d32d2d4e5) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x20000, "extra", 0 ) // how do we use this?!!
	ROM_LOAD( "mgraise.bin",  0x0000, 0x20000, CRC(019f37d4) SHA1(ab71fe0b41ff4415896a23f28b27a0e64950c68c) )


	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "mast5.bin",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "mast6.bin",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "mast7.bin",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "mast3.bin",  0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "mast4.bin",  0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "mast1.bin",  0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "mast2.bin",  0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "mast9.bin",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmasterd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cherryb.rom",  0x0000, 0x1000, CRC(b6ab94f6) SHA1(6e74a2354d15aa1da6b8207e0413158d7cb52a44) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmastere )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm33.rom",  0x0000, 0x1000, CRC(c3c3f7df) SHA1(47eda025859afebe64fd76e17e8390262fb40e0b) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmasterf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmbig55.rom",  0x0000, 0x1000, CRC(2cc4df7b) SHA1(ad5b8108913ff88fb435c8c12b47446575e1360e) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/* Cherry Master I (CM1-1.01)
   Sticker with "Cherry Master V4 -B-" on the PCB.
   Similar to cmasterb. Different program.

1x TMPZ84C00AP-6   u80    8-bit Microprocessor
1x D8255AC-5       u36    Programmable Peripheral Interface
1x D71055C         u54    Parallel Interface Unit
1x YM2149F         u13    Programmable Sound Generator
1x oscillator 12.000MHz   Y1

ROMs
4x 2764    1,2,3,4
4x 27256   5,6,7,9
1x 27512   8
3x N82S129AN    u46, u79, u84

RAMs
1x LC3517AL-10    u82
4x D4016CX-15-10  u22, u26, u28, u38

PLDs
4x PALCE16V8H-25pc/4    u51, u66, u73, u74

Others
1x 36x2 edge connector
1x 10x2 edge connector
1x pushbutton (SW6)
1x trimmer (volume)(VR1)
5x 8x2 switches DIP(SW1-SW5)
1x battery 5.5V(BT1)
*/
ROM_START( cmasterg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.u81",  0x0000,  0x1000, CRC(10979629) SHA1(a7342e37c75c85aef8a86ab5366a9e27f2c4bab8) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7.u16", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "6.u11", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "5.u4",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s129an.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "n82s129an.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s129an.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/* Cherry Master I (V1.10)
   Original Dyna upgrade for Cherry Master boards.
   The game runs in an original Dyna D9001 PCB.
   It lacks the STOP ALL button.

   From the owner (sic):
   - The reels cannot be stop, not exist all stop button or cannot be stop from small, big, double-up button.
   - The odds still be "high" (on the version 4.x be "low" odds).
   - Not hold a pair option to, and the min bet to start is fixed x1.
 */
ROM_START( cmasterh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm_v1.10_dyna.bin",  0x0000,  0x1000, CRC(872f2ef0) SHA1(ec68a03a1e8ab793d4a5eae1ce25f91608351c55) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7.u16", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "6.u11", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "5.u4",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s129an.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "n82s129an.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s129an.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmast99 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm99-041-8.u81", 0x0000, 0x1000, CRC(5fb0800e) SHA1(14d316b7b89340c5b5c9fdd0b43d5810513b74d6) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cm99-041-7.u16", 0x00000, 0x8000, CRC(69e2aef2) SHA1(195faec239734650dcd777d55a8da84e3a0ed50c) )
	ROM_LOAD( "cm99-041-6.u11", 0x08000, 0x8000, CRC(900f36f5) SHA1(0fd41f8c8cb2f7940b653a1fad93df2e3f28a34b) )
	ROM_LOAD( "cm99-041-5.u4",  0x10000, 0x8000, CRC(3e465e38) SHA1(847dc27e45d495cb924b3fd5ce8e68a1cb83ffc8) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "cm99-041-4.u15", 0x0000, 0x2000, CRC(6b870b29) SHA1(d65f24817d9d45c148cb857439b46e9e75dabfe7) )
	ROM_LOAD( "cm99-041-3.u10", 0x2000, 0x2000, CRC(8a0b205f) SHA1(3afea0464b793526bf23610cac6736a31edc7ec2) )
	ROM_LOAD( "cm99-041-2.u14", 0x4000, 0x2000, CRC(c84dba45) SHA1(ab4ac891a23d6b9a216df046d516e868c77e8a36) )
	ROM_LOAD( "cm99-041-1.u9",  0x6000, 0x2000, CRC(44046c31) SHA1(c9703ce2371cf86bc597e5fdb9c0d4dd6d91f7dc) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_FILL( 0x0000, 0x10000, 0xff ) // U53 (girl bitmaps) not populated

	/* proms taken from cmv4, probably wrong; U79? and U84 known to be identical to Cherry Master 89 */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END


ROM_START( cmast99b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm99b-041-8.u81", 0x0000, 0x1000, CRC(e0872d9f) SHA1(6d8f5e09e5c9daf834d5c74434eae86e5dd7e194) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cm99-041-7.u16", 0x00000, 0x8000, CRC(69e2aef2) SHA1(195faec239734650dcd777d55a8da84e3a0ed50c) )
	ROM_LOAD( "cm99-041-6.u11", 0x08000, 0x8000, CRC(900f36f5) SHA1(0fd41f8c8cb2f7940b653a1fad93df2e3f28a34b) )
	ROM_LOAD( "cm99-041-5.u4",  0x10000, 0x8000, CRC(3e465e38) SHA1(847dc27e45d495cb924b3fd5ce8e68a1cb83ffc8) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "cm99-041-4.u15", 0x0000, 0x2000, CRC(6b870b29) SHA1(d65f24817d9d45c148cb857439b46e9e75dabfe7) )
	ROM_LOAD( "cm99-041-3.u10", 0x2000, 0x2000, CRC(8a0b205f) SHA1(3afea0464b793526bf23610cac6736a31edc7ec2) )
	ROM_LOAD( "cm99-041-2.u14", 0x4000, 0x2000, CRC(c84dba45) SHA1(ab4ac891a23d6b9a216df046d516e868c77e8a36) )
	ROM_LOAD( "cm99-041-1.u9",  0x6000, 0x2000, CRC(44046c31) SHA1(c9703ce2371cf86bc597e5fdb9c0d4dd6d91f7dc) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_FILL( 0x0000, 0x10000, 0xff ) // U53 (girl bitmaps) not populated

	/* proms taken from cmv4, probably wrong; U79? and U84 known to be identical to Cherry Master 89 */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END

/*
    A-Plan
    (C) 1993 WeaShing H.K.

    TMP91P640 @ 5MHz or 10MHz (or SDIP64 gfx chip of some kind?)
    Z80 [clock probably 12/4]
    8255 x2
    YM2149 [clock probably 12/8]
    12MHz and 10MHz XTALs
    6116 2kx8 SRAM x4
    BPROM 82S129 x2
    8-position DIPSW x5
    PCB number only says 'WEASHING'
*/

ROM_START( aplan ) // Has "DYNA QL-1  V1.01" string.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u18", 0x02000, 0x1000, CRC(e0e5328d) SHA1(fd02c65fecada69a0953372a39e520a80c744109) )
	ROM_CONTINUE(0xf000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u64",   0x00000, 0x8000, CRC(2e593152) SHA1(ca0f587b26ed6e962e100d9178b6f79995a26fec) )
	ROM_LOAD( "u67",   0x08000, 0x8000, CRC(aa865924) SHA1(574c4ba06e0bb37624ebd0fd7f5bfa4d0be62eba) )
	ROM_LOAD( "u68",   0x10000, 0x8000, CRC(3043a99c) SHA1(cf936a866cc90b1cb477914e484e1d21165d6526) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* ??? */
	ROM_LOAD( "u56",   0x0000, 0x4000, CRC(e1ea0658) SHA1(1f31c493556f6ff6564be439040914a18a56ea64) )
	ROM_LOAD( "u58",   0x4000, 0x4000, CRC(aa5ae1a5) SHA1(f4a9b3d97ac4df81fc071962698eab4b63e2e608) )
	ROM_LOAD( "u60",   0x8000, 0x4000, CRC(ba6cc02d) SHA1(80f06761eeb08b3852041f7a1773059c69420c44) )
	ROM_LOAD( "u62",   0xc000, 0x4000, CRC(609a6300) SHA1(ec8301c603a2af2aeecbef83d5cfba1796209988) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_FILL( 0x0000, 0x10000, 0xff ) // (girl bitmaps, not present)

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u45",   0x000, 0x100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u46",   0x100, 0x100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )
ROM_END

ROM_START( chryangl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.u6",  0x0000, 0x10000, CRC(331961e4) SHA1(50c7e0e983aed1f199f238442bb8fafce1849f84) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "7.e1",  0x00000,  0x10000, CRC(f8e523ba) SHA1(bfbe1803f0f3c3426a4cc252257c8a4dd83a70ac) )
	ROM_LOAD( "6.e2",  0x10000,  0x10000, CRC(0d3b322a) SHA1(64b6bd387a78f51f83002c67d857b157a4651279) )
	ROM_LOAD( "5.e3",  0x20000,  0x10000, CRC(da87dbeb) SHA1(3656b569d08540171003820ec86944d2a7a55b3b) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.f3",  0x0000,  0x2000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "2.f2",  0x2000,  0x2000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "3.h3",  0x4000,  0x2000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "4.h2",  0x6000,  0x2000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	// nothing

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129a.c15", 0x0000, 0x0100, CRC(6144d7fc) SHA1(4563ea31864d8732e3a4b0270449a0a79db334a2) )
	ROM_LOAD( "82s129a.c16", 0x0100, 0x0100, CRC(0893e05d) SHA1(fabd58d498f5efdddae4c7142915cb7b092d6804) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.e9", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( tonypok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tonypok.rom",  0x0000,  0x1000, CRC(c7047fcb) SHA1(a224e5a3c0fcd1d588ab264c4d0c624159834488) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "pok_u16.7", 0x00000,  0x8000, CRC(d7511644) SHA1(2aedd80b279f6e1231bacfce913e06070c74fff7) )
	ROM_LOAD( "pok_u11.6", 0x08000,  0x8000, CRC(6ff4d0f9) SHA1(3faccac9562c9269f392655d045a10569f335ccc) )
	ROM_LOAD( "pok_u4.5",  0x10000,  0x8000, CRC(7c641db2) SHA1(b90d4c5efc388fe8938ed3180b3c36a20ecdc15b) )

	// the remainder of the roms are from Cherry Master - this was a conversion kit
	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

// the program roms on these seem scrambled somehow
ROM_START( jkrmast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pid-515.u5",  0x0000, 0x10000, CRC(73caf824) SHA1(b7a7bb6190465f7c3b40f2ef97f4f6beeb89ec41) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2000b.u48", 0x00000,  0x20000, CRC(e7b406ec) SHA1(c0a10cf8bf5467ecfe3c90e6897db3ab9aae0127) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2000a.u41", 0x00000,  0x20000, CRC(cb8b1563) SHA1(c8c3ae646a9f3a7482d83566e4b3e18441c5d67f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s147a.u13", 0x0000, 0x0200, CRC(da92f0ae) SHA1(1269a2029e689a5f111c57e80825b3756b50521e) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s129.u28",  0x0000, 0x0100, CRC(cfb152cf) SHA1(3166b9b21be4ce1d3b6fc8974c149b4ead03abac) )
ROM_END

ROM_START( jkrmasta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pid-513.u5",  0x0000, 0x10000, CRC(12fa7ea0) SHA1(71ee141fe01ae2ce9913620b52c54cf445fd0b00) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2000b.u48", 0x00000,  0x20000, CRC(e7b406ec) SHA1(c0a10cf8bf5467ecfe3c90e6897db3ab9aae0127) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2000a.u41", 0x00000,  0x20000, CRC(cb8b1563) SHA1(c8c3ae646a9f3a7482d83566e4b3e18441c5d67f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s147a.u13", 0x0000, 0x0200, CRC(da92f0ae) SHA1(1269a2029e689a5f111c57e80825b3756b50521e) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s129.u28",  0x0000, 0x0100, CRC(cfb152cf) SHA1(3166b9b21be4ce1d3b6fc8974c149b4ead03abac) )
ROM_END

ROM_START( pkrmast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokermastera.rom",  0x0000, 0x10000, CRC(467249f7) SHA1(efbab56896dc58d22ec921e7f5fd0befcfaadc52) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // tiles
	ROM_LOAD( "103-2.bin", 0x00000,  0x20000, CRC(ed0dfbfe) SHA1(c3a5b68e821461b161293eaec1515e2b0f26c4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // reels + girl?
	ROM_LOAD( "103-1.bin", 0x00000,  0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END


ROM_START( pkrmasta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokermasterb.rom",  0x0000, 0x10000, CRC(f59e0273) SHA1(160426b86dbb8a718cb3b886f90a231baed86a40) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // tiles
	ROM_LOAD( "103-2.bin", 0x00000,  0x20000, CRC(ed0dfbfe) SHA1(c3a5b68e821461b161293eaec1515e2b0f26c4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // reels + girl?
	ROM_LOAD( "103-1.bin", 0x00000,  0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) BAD_DUMP )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) BAD_DUMP )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) BAD_DUMP )
ROM_END


/*

Cherry Master '91
-----------------

CPU: z8400
other chips: NEC D71055C, WB5300
Dips 5 x 8 position

OSC: 12.000mhz

all pals are type 16L8
all proms are type s129

*/
ROM_START( cmast91 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.bin",   0x00000, 0x01000, CRC(31a16d9f) SHA1(f007148449d66954b780f12a9f910968a4052482) )
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x8000, CRC(1edf1f1d) SHA1(558fa01f1efd7f6541047d3930bdce0974bae5b0))
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(13582e74) SHA1(27e318542606b8e8d38250749ba996402d314abd) )
	ROM_LOAD( "5.bin",  0x10000, 0x8000, CRC(28ff88cc) SHA1(46bc0407be857e8348159735b60cfb660f047a56) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.bin",  0x00000, 0x8000, CRC(0dbabaa2) SHA1(44235b19dac1c996e2166672b03f6e3888ecbefa) )
	ROM_LOAD( "3.bin",  0x08000, 0x8000, CRC(dc77d04a) SHA1(d8656130cde54d4bb96307899f6d607867e49e6c) )
	ROM_LOAD( "1.bin",  0x10000, 0x8000, CRC(71bdab69) SHA1(d2c594ed88d6368df15b623c48eecc1c219b839e) )
	ROM_LOAD( "2.bin",  0x18000, 0x8000, CRC(fccd48d7) SHA1(af564f5ef9ff5b6363897ce6bdf0b21123911fd4) )

	ROM_REGION( 0x40000, "user1", 0 ) /* unknown, bitmaps, or sound? */
	ROM_LOAD( "9.bin",  0x00000, 0x40000, CRC(92342276) SHA1(f9436752f2ec67cf873fd01c729c7c113dc18be0) ) // ?

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "p1.bin", 0x0000, 0x0100, CRC(ac529f04) SHA1(5bc92e50c85bb23e609172cc15c430ddea7fdcb5) )
	ROM_LOAD( "p2.bin", 0x0100, 0x0100, CRC(3febce95) SHA1(c7c0fec0fb024ebf7d7365a09d28ba3d0037b0b4) )
	ROM_LOAD( "p3.bin", 0x0200, 0x0100, CRC(99dbdf19) SHA1(3680335406f63289f8d9a81b4cd163e4aa0c14d4) )

	ROM_REGION( 0x100, "proms2", 0 ) /* screen layout? */
	ROM_LOAD( "p4.bin", 0x0000, 0x0100, CRC(72212427) SHA1(e87a91f28284313c706ebb8175a3586780636e31) )

	ROM_REGION( 0x800, "plds", 0 ) /* all 16L8 type, protected */
	ROM_LOAD( "pld1.bin", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pld2.bin", 0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "pld3.bin", 0x0400, 0x0104, NO_DUMP )
	ROM_LOAD( "pld4.bin", 0x0600, 0x0104, NO_DUMP )
ROM_END


ROM_START( cmast92 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm9230d.rom",   0x00000, 0x01000, CRC(214a0a2d) SHA1(2d349e0888ac2da3df954517fdeb9214a3b17ae1) )
	// I've not checked the rom loading yet
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	// we only have a program rom :-(
	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cherry master 92 graphics",  0x00000, 0x8000, NO_DUMP )
	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF )
	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF )
	ROM_REGION( 0x300, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "cherry master 92 proms", 0x00000, 0x100, NO_DUMP )
	ROM_REGION( 0x100, "proms2", ROMREGION_ERASEFF )
ROM_END


/*

        Lucky 8 Line
        Falcon 1989

        G14           6116  9
        G13   D13           8
              D12
        6116                 Z80
        6116                 8255
        7                    8255
        6            SW1     8255
 12MHz  5            SW2     8910
        4  6116      SW4
        3  6116      SW3
        2  6116
        1  6116

    ---

*/
ROM_START( lucky8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8",   0x0000, 0x4000, CRC(a187573e) SHA1(864627502025dbc83a0049fc98505655cec7b181) )
	ROM_LOAD( "9",   0x4000, 0x4000, CRC(6f62672e) SHA1(05662ef1a70f93b09e48de497b049a282f070735) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*

unknown korean or chinese bootleg of something?

XTAL 12MHz
Z80 @ 3MHz
AY3-8910 @ 1.5MHz
8255 x3
RAM 6116 x5
76489 x1
8-position DSW x4

----

13 and 13 files
g14                                             FIXED BITS (0000xxxx)
g14                                             BADADDR                xxxxx---
g13                                             FIXED BITS (1x1xxxxx11xxxxxx)
d13                                             FIXED BITS (xxxxxx0xxxxxxxxx)
d13                                             1ST AND 2ND HALF IDENTICAL
d12                                             FIXED BITS (0000xxxx)
                        prom1                   FIXED BITS (xxxxxx0xxxxxxxxx)
                        prom1                   1ST AND 2ND HALF IDENTICAL
                        prom2                   FIXED BITS (1x11xxxx11x1xxxx)
                        prom3                   FIXED BITS (0000xxxx)
                        prom4                   FIXED BITS (0000xxxx)
                        prom5                   FIXED BITS (00001xxx)
                        prom5                   BADADDR                xxxxxxx-
d13                     prom1                   IDENTICAL
d12                     prom3                   IDENTICAL
6                       7                       IDENTICAL
5                       6                       IDENTICAL
4                       5                       IDENTICAL
3                       4                       IDENTICAL
2                       3                       IDENTICAL
1                       2                       IDENTICAL
7                       8                       99.990845%
g13                     prom2                   90.625000%
g14                     prom4                   61.718750%
9                                               NO MATCH
8                                               NO MATCH
                        1                       NO MATCH
                        prom5                   NO MATCH

There is a loop at 0x0010 that decrement (HL) when is pointing to ROM space.
This should be worked out or patched to allow boot the game.
Seems to be related to timing since once patched the game is very fast.

*/
ROM_START( lucky8a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// we have to patch this, it might be bad
	ROM_LOAD( "1",  0x0000, 0x8000, BAD_DUMP CRC(554cddff) SHA1(8a0678993c7010f70adc9e9443b51cf5929bf110) ) // sldh

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "6",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) ) // sldh
	ROM_LOAD( "7",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) ) // sldh
	ROM_LOAD( "8",  0x10000, 0x8000, CRC(80b35f06) SHA1(561d257d7bc8976cfa08f36d84961f1263509b5b) ) // sldh

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2",  0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) ) // sldh
	ROM_LOAD( "3",  0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) ) // sldh
	ROM_LOAD( "4",  0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) ) // sldh
	ROM_LOAD( "5",  0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) ) // sldh

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "prom3", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom1", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "prom5", 0x000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "prom2", 0x0000, 0x0020, CRC(7b1a769f) SHA1(788b3573df17d398c74662fec4fd7693fc27e2ef) )
ROM_END


/*
   New Lucky 8 Lines (set 3, extended gfx)

  This set has the New Lucky 8 Lines / New Super 8 Lines program.
  Same extended tileset for reels, but lacks of the New Super 8 Lines title tiles.
  Maybe is a hidden feature, maybe just graphics for another hack.

*/
ROM_START( lucky8b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.bin",  0x0000, 0x8000, CRC(ab7c58f2) SHA1(74782772bcc91178fa381074ddca99e0515f7693) ) // sldh

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",  0x00000, 0x8000, CRC(9bbe5437) SHA1(ef3789cf34d83827bd6ad4755fd443c3d9bdf661) )
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(bc17a96b) SHA1(6ae6a99c72153d68b01feacc45d94f8f88ac8733) )
	ROM_LOAD( "7.bin",  0x10000, 0x8000, CRC(06a98714) SHA1(e58efdcbdc021976d5a1253c03bea0bfad4d92db) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.bin",  0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.bin",  0x4000, 0x2000, CRC(b4e58020) SHA1(5c0fcc4b5d484ca7de5f2bd568a391a45967a9cc) )
	ROM_LOAD( "4.bin",  0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "u4.bin", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "u5.bin", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "u2.bin", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


ROM_START( lucky8c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.bin",   0x0000, 0x8000, CRC(6890f8d8) SHA1(7e9d974acf199c78972299bfa3e275a30a3f6eaa) ) // sldh

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


ROM_START( lucky8d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8-40_.bin",   0x0000, 0x4000, CRC(4c79db5a) SHA1(b959030856f54776841092c4c2bccc6565faa587) )
	ROM_LOAD( "9-40_.bin",   0x4000, 0x4000, CRC(fb0d511f) SHA1(c2c1868339d4f20bf1f5d6b66802e8f8deed4611) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*

Z80
3x 8255
ay-38910A
3 prom (no dump right now)

27256.8 is Z80 PRG

27256.8             NO MATCH

27128.1             NO MATCH
27128.7             NO MATCH
27256.5             = 5                     lucky8     New Lucky 8 Lines (set 1, W-4)
                    = 6                     lucky8a    New Lucky 8 Lines (set 2, W-4)
                    = 5                     lucky8c    New Lucky 8 Lines (set 4, W-4)
                    = 5                     lucky8d    New Lucky 8 Lines (set 5, W-4)
27256.6             = 6                     lucky8     New Lucky 8 Lines (set 1, W-4)
                    = 7                     lucky8a    New Lucky 8 Lines (set 2, W-4)
                    = 6                     lucky8c    New Lucky 8 Lines (set 4, W-4)
                    = 6                     lucky8d    New Lucky 8 Lines (set 5, W-4)
2764.2              = 2.bin                 ladylinr   Lady Liner
                    = 2                     lucky8     New Lucky 8 Lines (set 1, W-4)
                    = 3                     lucky8a    New Lucky 8 Lines (set 2, W-4)
                    = 2                     lucky8c    New Lucky 8 Lines (set 4, W-4)
                    = 2                     lucky8d    New Lucky 8 Lines (set 5, W-4)
2764.3              = 3.bin                 ladylinr   Lady Liner
                    = 3                     lucky8     New Lucky 8 Lines (set 1, W-4)
                    = 4                     lucky8a    New Lucky 8 Lines (set 2, W-4)
                    = 3                     lucky8c    New Lucky 8 Lines (set 4, W-4)
                    = 3                     lucky8d    New Lucky 8 Lines (set 5, W-4)
2764.4              = 4.bin                 ladylinr   Lady Liner
                    = 4                     lucky8     New Lucky 8 Lines (set 1, W-4)
                    = 5                     lucky8a    New Lucky 8 Lines (set 2, W-4)
                    = 4                     lucky8c    New Lucky 8 Lines (set 4, W-4)
                    = 4                     lucky8d    New Lucky 8 Lines (set 5, W-4)

...and against lucky8:

                        27128.1                 1ST AND 2ND HALF IDENTICAL
                        27128.7                           11xxxxxxxxxxxx = 0xFF
6                       27256.6                 IDENTICAL
5                       27256.5                 IDENTICAL
4                       2764.4                  IDENTICAL
3                       2764.3                  IDENTICAL
2                       2764.2                  IDENTICAL
7            [2/2]      27128.7                 IDENTICAL
1                       27128.1      [1/2]      IDENTICAL
1                       27128.1      [2/2]      IDENTICAL
9            [1/2]      27256.8      [3/4]      IDENTICAL
8            [2/2]      27256.8      [2/4]      99.731445%
8            [1/2]      27256.8      [1/4]      98.413086%
9            [2/2]      27256.8      [4/4]      90.710449%

The program is exactly the same of lucky8d, with 40% for main rate and 60% for d-up,
but merged in only one 27128 EPROM instead of two.

*/
ROM_START( lucky8e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256.8",   0x0000, 0x8000, CRC(65decc53) SHA1(100f26ef796557182ba894d1e30b18ac58a793be) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "27256.6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
//  ROM_LOAD( "27128.7",  0x10000, 0x4000, BAD_DUMP CRC(0000b9d0) SHA1(00008fe8a116c33bbd712a639224d041447a45c1) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) ) // from parent set, since 2 of 3 bitplanes matched

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "27128.1",  0x0000, 0x2000, CRC(8ca19ee7) SHA1(2e0cd4a74bd9abef60ed561ba4e5bb2681ce1222) )    // overdump?
	ROM_IGNORE(                   0x2000)
	ROM_LOAD( "2764.2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "2764.3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "2764.4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  New Lucky 8 Lines / New Super 8 Lines.

  This set has a regular New Lucky 8 Lines, but allowing bets up to 64.
  It has different settings/parameters, and additional graphics for a game
  called New Super 8 Lines. There are basic reels tiles with a semi-naked woman,
  a sort of Super Mario character from Nintendo, clouds and stars...


   PCB Layout:
  +----------------------------------------------------------------------------------------------------------------------------+
  |      J       I       H         G            F         E         D           C          B              A                    |
  |                                                                        +----------+                                        |
  |   +-----+ +-----+ +-------+ +-----+                +-----+             |ASB IN USA| +--------------+ +-----+            +--+
  |14 |  U  | |  U  | |   F   | |  T  |                |  M  |             |4FJ       | |      8       | |  R  |            |
  |   +-----+ +-----+ +-------+ +-----+                +-----+             |HM6116LP-2| |   D27256     | +-----+            |
  |                                                                        +----------+ |              |                    +--+
  |   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+    +--------------+ +-----+             --|
  |13 |  H  |           |  V  | |  T  |      |  H  |   |   F   | |  S  |   |   A   |                     |  M  |             --|
  |   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+                     +-----+             --|
  |                +----------+                                                                                              --|
  |                |Toshiba   | +-----+                +-----+   +-----+   +-------+                                         --|
  |12              |TMM2016BP-| |  O  |                |  B  |   |  S  |   |   A   |                                         --|
  |                |12        | +-----+                +-----+   +-----+   +-------+                                         --|
  |                +----------+                                                                                              --|
  |                +----------+                                            +------------------+                              --|
  |                |          | +-----+                +-----+   +-----+   |      ZILOG       |                              --|
  |11              |HM6116L-90| |  O  |                |  B  |   |  L  |   |   Z0840004PSC    |                              --|
  |                |          | +-----+                +-----+   +-----+   |   Z80 CPU        |                              --|
  |                +----------+                                            +------------------+                              --|
  |   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
  |10 |  J  |  |      [7]     | |  O  |      |  W  |   |  B  |   |  X  |   |    NEC JAPAN     |                              --|
  |   +-----+  |    D27256    | +-----+      +-----+   +-----+   +-----+   |    D8255AC-2     |                              --|
  |            |              |                                            |    9014XD010     |                              --|
  |   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                            36--|
  |9  |  J  |  +--------------+ |  O  |      |  E  |   |  C  |   |  L  |                                                 Pinout|
  |   +-----+  |      [6]     | +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
  |   +-----+  |    D27256    | +-------+    +-----+   +-----+ +--------+  |    NEC JAPAN     |                              --|
  |8  |  J  |  |              | |   I   |    |  E  |   |  D  | |  DIP1  |  |    D8255AC-2     |                              --|
  |   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  |    9014XD010     |                              --|
  |   +-----+  +--------------+ +-------+              +-----+ +--------+  +------------------+ +---+ +-----+                --|
  |7  |  G  |  |      [5]     | |   I   |              |  E  | |  DIP2  |  +------------------+ | Q | |  P  |                --|
  |   +-----+  |   D27256     | +-------+              +-----+ +--------+  |    NEC JAPAN     | +---+ +-----+                --|
  |            |              |                                            |                  |                              --|
  |    XTAL    +--------------+ +-------+              +-----+ +--------+  |                  |       +-----+                --|
  |6  .----.   +--------------+ |   I   |              |  L  | |  DIP3  |  +------------------+       |  P  |                --|
  |            |      [4]     | +-------+              +-----+ +--------+  +------------------+       +-----+                --|
  |            |   D2764D     |                                            |     Winbond      |                              --|
  |   +-----+  |              | +-------+    +-----+   +-----+ +--------+  |     WF19054      |                              --|
  |5  |  J  |  +--------------+ |   I   |    |  K  |   |  B  | |  DIP4  |  |  4150C14090830   |                              --|
  |   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  +------------------+                              --|
  |            |      [3]     | +----------+                                                                                 --|
  |   +-----+  |  HN482764G   | |Toshiba Tm| +-----+   +-----+ +---------+  +------+ +------+                                --|
  |4  |  J  |  |              | |m2016BP-12| |  K  |   |  B  | |    I    |  |   V  | |   X  |                                --|
  |   +-----+  +--------------+ +----------+ +-----+   +-----+ +---------+  +------+ +------+                               +--+
  |   +-----+  +--------------+ +----------+ +-----+   +-----+ +-------+    +------+ +------+                               |
  |3  |  J  |  |     [2]      | |USC 6516-A| |  T  |   |  B  | |   H   |    |   V  | |   X  |                               |
  |   +-----+  |    D2764     | |9252E GYU1| +-----+   +-----+ +-------+    +------+ +------+                               +--+
  |            |              | +----------+                                                                                   |
  |   +-----+  +--------------+ +----------+ +-----+   +-----+  +------+    +------+                                           |
  |2  |  J  |  +--------------+ |Toshiba Tm| |  O  |   |  M  |  |   Y  |    |   Z  |                                           |
  |   +-----+  |     [1]      | |m2016BP-12| +-----+   +-----+  +------+    +------+                                           |
  |   +-----+  |  MBM2764-25  | +----------+ +-----+   +-----+  +------+                                                       |
  |1  |  J  |  |              | +----------+ |  O  |   |  N  |  |   W  |                                                       |
  |   +-----+  +--------------+ |HM6116L-90| +-----+   +-----+  +------+                                  A                    |
  |                             |  9140A   |                                           +-----+    10 Pins   +-----+            |
  |      J       I       H      +----------+    F         E         D           C     B|     ||||||||||||||||     |            |
  +------------------------------------------------------------------------------------+     +--------------+     +------------+

  DIP1:                     DIP2:                     DIP3:                     DIP4:
  +-------------------+     +-------------------+     +-------------------+     +-------------------+
  | ON                |     | ON                |     | ON                |     | ON                |
  | +---------------+ |     | +---------------+ |     | +---------------+ |     | +---------------+ |
  | |_|_|_|#|_|_|_|_| |     | |#|#|#|_|_|_|_|#| |     | |_|_|#|#|#|#|#|_| |     | |_|_|_|_|#|_|_|_| |
  | |#|#|#| |#|#|#|#| |     | | | | |#|#|#|#| | |     | |#|#| | | | | |#| |     | |#|#|#|#| |#|#|#| |
  | +---------------+ |     | +---------------+ |     | +---------------+ |     | +---------------+ |
  |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |
  +-------------------+     +-------------------+     +-------------------+     +-------------------+

  1x XTAL = 12 Mhz

  A = SN74LS244N
  B = GD74LS161A
  C = MB74LS10
  D = 74LS10
  E = GD74LS74A
  F = HD74LS273P
  G = HD74LS368AP
  H = 74LS32
  I = 74LS245
  J = GD74LS166
  K = SN74LS283N
  L = GD74LS138
  M = GD74LS04
  N = SN74LS139AN
  O = GD74LS157
  P = HD74LS04P
  Q = 5560 / JRC / 3151A (555?)
  R = HD74HC00P
  S = DM74S288
  T = DM74S287
  U = LS02
  V = LS174
  W = LS08
  X = LS367
  Y = LS00
  Z = SN76489AN

*/
ROM_START( ns8lines )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.bin",  0x0000, 0x8000, CRC(ab7c58f2) SHA1(74782772bcc91178fa381074ddca99e0515f7693) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",  0x00000, 0x8000, CRC(994a9894) SHA1(4063c2c5e111f24a85df1665fd3f9fbb20fda4da) )
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(80888d64) SHA1(91ec96709df77c534d381e391839984a88aeb1e0) )
	ROM_LOAD( "7.bin",  0x10000, 0x8000, CRC(255d5860) SHA1(f171fde3d542594132b38b44300f750d45fb67a2) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.bin",  0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.bin",  0x4000, 0x2000, CRC(6be213c8) SHA1(bf5a002961b0827581cbab4249321ae5b51316f0) )
	ROM_LOAD( "4.bin",  0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "u4.bin", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "u5.bin", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "u2.bin", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  New Lucky 8 Lines / New Super 8 Lines (F-5, Witch Bonus)

  This set has the 'Witch Bonus' present in Witch Card games.

  1.1h         [1/2]      1    [1/2]      IDENTICAL
  2.3h         [1/2]      2    [1/2]      IDENTICAL
  3.4h         [1/2]      3    [1/2]      IDENTICAL
  4.5h         [1/2]      4    [1/2]      IDENTICAL
  5.7h         [1/4]      5    [1/4]      IDENTICAL
  6.8h         [1/4]      6    [1/4]      IDENTICAL
  7.10h        [1/4]      7    [1/4]      IDENTICAL
  7.10h        [3/4]      7    [3/4]      IDENTICAL
  7.10h        [2/4]      7    [2/4]      99.426270%
  6.8h         [2/4]      6    [2/4]      99.255371%
  5.7h         [2/4]      5    [2/4]      99.230957%
  5.7h         [3/4]      5    [3/4]      94.152832%
  6.8h         [3/4]      6    [3/4]      94.152832%
  7.10h        [4/4]      7    [4/4]      82.739258%
  7.10h        [4/4]      6    [4/4]      82.739258%
  7.10h        [4/4]      5    [4/4]      82.739258%
  4.5h         [2/2]      4    [2/2]      81.372070%
  6.8h         [4/4]      9    [2/2]      75.952148%
  3.4h         [2/2]      3    [2/2]      71.752930%
  2.3h         [2/2]      2    [2/2]      70.434570%
  1.1h         [2/2]      1    [2/2]      68.115234%
  f5-8.14b     [4/4]      8    [1/2]      8.276367%
  5.7h         [4/4]      9    [1/2]      7.104492%
  f5-8.14b     [3/4]      8    [2/2]      1.904297%

*/
ROM_START( ns8linew )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f5-8.14b",   0x0000, 0x8000, CRC(63dd3005) SHA1(62d71dbfa0a00c6b050db067ad55e80225e1589d) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.7h",  0x00000, 0x8000, CRC(19713d36) SHA1(ae04b8b72b0c9a279f24d7c4d619bac4629d9a4f) )
	ROM_LOAD( "6.8h",  0x08000, 0x8000, CRC(576197b9) SHA1(22273365cfe181f95efb895a28825f388b901a49) )
	ROM_LOAD( "7.10h", 0x10000, 0x8000, CRC(790c349c) SHA1(32bec8463233b2eb5a889c91d35f53b9d117f279) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.1h",  0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.3h",  0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.4h",  0x4000, 0x2000, CRC(b4e58020) SHA1(5c0fcc4b5d484ca7de5f2bd568a391a45967a9cc) )
	ROM_LOAD( "4.5h",  0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "dm74s287.g13", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "dm74s287.g14", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dm74s288.d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "dm74s287.f3",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "dm74s288.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  New Lucky 8 Lines / New Super 8 Lines (W-4, Witch Bonus)

  CPUs
  1x Z80        (11b) - 8-bit Microprocessor - main (missing).
  3x iD8255A-5  (7b, 8b, 9b) - Programmable Peripheral Interface.
  1x SN76489AN  (2c) - Digital Complex Sound Generator - sound.
  1x WF19054    (5b) - Programmable Sound Generator - sound.
  1x uPC1181    (0a) - Audio Amplifier - sound.

  ROMs
  4x AM27C64    (1-4)   dumped.
  4x M27C256B   (5-8) dumped.
  3x AM27S21APC (1, 2, 3) dumped.
  3x N82S123AN  (4, 5) dumped.

  RAMs
  7x HM3-6116-5 (11h, 12h, 1g, 2g, 3g, 4g, 14c)

  Others
  1x 12.000MHz oscillator (6j).
  1x 36x2 edge connector.
  1x 10x2 edge connector.
  1x pushbutton.
  2x trimmer (VOL, VAL).
  3x 8 DIP switches banks.


   PCB Layout:
  +----------------------------------------------------------------------------------------------------------------------------+
  |      J       I       H         G            F         E         D           C          B              A           push     |
  |                                                                        +----------+                               button   |
  |   +-----+ +-----+ +-------+ +-----+                +-----+             |RAM 6116  | +--------------+ +-----+            +--+
  |14 |  U  | |  U  | |   F   | |  T  |                |  M  |             |          | |     [8]      | |  R  |            |
  |   +-----+ +-----+ +-------+ +-----+                +-----+             |(missing) | |   M27C256B   | +-----+            |
  |                                                                        +----------+ |              |                    +--+
  |   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+    +--------------+ +-----+             --|
  |13 |  H  |           |  V  | |  T  |      |  H  |   |   F   | |  S  |   |   A   |                     |  P  |             --|
  |   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+                     +-----+             --|
  |                +----------+                                                                                              --|
  |                |TEMIC     | +-----+                +-----+   +-----+   +-------+                                         --|
  |12              |HM3-6116-5| |  O  |                |  B  |   |  S  |   |   A   |                                         --|
  |                |          | +-----+                +-----+   +-----+   +-------+                                         --|
  |                +----------+                                                                                              --|
  |                +----------+                                            +------------------+                              --|
  |                |TEMIC     | +-----+                +-----+   +-----+   |     Z80 CPU      |                              --|
  |11              |HM3-6116-5| |  O  |                |  B  |   |  L  |   |    (missing)     |                              --|
  |                |          | +-----+                +-----+   +-----+   |                  |                              --|
  |                +----------+                                            +------------------+                              --|
  |   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
  |10 |  J  |  |     [7]      | |  O  |      |  W  |   |  B  |   |  X  |   |    INTEL         |                              --|
  |   +-----+  |   M27C256B   | +-----+      +-----+   +-----+   +-----+   |    P8255A-5      |                              --|
  |            |              |                                            |                  |                              --|
  |   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                            36--|
  |9  |  J  |  +--------------+ |  O  |      |  E  |   |  C  |   |  L  |                                                 Pinout|
  |   +-----+  |     [6]      | +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
  |   +-----+  |   M27C256B   | +-------+    +-----+   +-----+ +--------+  |    INTEL         |                              --|
  |8  |  J  |  |              | |   I   |    |  E  |   |  D  | |  DIP1  |  |    P8255A-5      |                              --|
  |   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  |                  |                              --|
  |   +-----+  +--------------+ +-------+              +-----+ +--------+  +------------------+ +---+ +-----+                --|
  |7  |  G  |  |     [5]      | |   I   |              |  E  | |  DIP2  |  +------------------+ | Q | |  P  |                --|
  |   +-----+  |   M27C256B   | +-------+              +-----+ +--------+  |    INTEL         | +---+ +-----+                --|
  |            |              |                                            |    P8255A-5      |                              --|
  |    XTAL    +--------------+ +-------+              +-----+             |                  |       +-----+                --|
  |6  .----.   +--------------+ |   I   |              |  L  |             +------------------+       |  P  |                --|
  |            |     [4]      | +-------+              +-----+             +------------------+       +-----+                --|
  |            |   AM27C64    |                                            |     Winbond      |                              --|
  |   +-----+  |              | +-------+    +-----+   +-----+ +--------+  |     WF19054      |                              --|
  |5  |  O  |  +--------------+ |   I   |    |  K  |   |  B  | |  DIP3  |  |                  |                              --|
  |   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  +------------------+                              --|
  |            |     [3]      | +----------+                                                                                 --|
  |   +-----+  |   AM27C64    | |TEMIC     | +-----+   +-----+ +---------+  +------+ +------+                                --|
  |4  |  J  |  |              | |HM3-6116-5| |  K  |   |  B  | |    I    |  |   V  | |   X  |                                --|
  |   +-----+  +--------------+ +----------+ +-----+   +-----+ +---------+  +------+ +------+                               +--+
  |   +-----+  +--------------+ +----------+ +-----+   +-----+ +-------+    +------+ +------+                               |
  |3  |  J  |  |     [2]      | |TEMIC     | |  T  |   |  E  | |   H   |    |   V  | |   X  |                               |
  |   +-----+  |   AM27C64    | |HM3-6116-5| +-----+   +-----+ +-------+    +------+ +------+                               +--+
  |            |              | +----------+                                                                                   |
  |   +-----+  +--------------+ +----------+ +-----+   +-----+  +------+    +------+     trimmer        trimmer                |
  |2  |  J  |  +--------------+ |TEMIC     | |  O  |   |  M  |  |   Y  |    |   Z  |     VAL 10K        VOL 10K                |
  |   +-----+  |     [1]      | |HM3-6116-5| +-----+   +-----+  +------+    +------+                                           |
  |   +-----+  |   AM27C64    | +----------+ +-----+   +-----+  +------+                                            +-------+  |
  |1  |  J  |  |              | +----------+ |  O  |   |  N  |  |   W  |                                            |NEC    |  |
  |   +-----+  +--------------+ |TEMIC     | +-----+   +-----+  +------+                                  A         |C1181H3|  |
  |                             |HM3-6116-5|                                           +-----+    10 Pins   +-----+ +-------+  |
  |      J       I       H      +----------+    F         E         D           C     B|     ||||||||||||||||     |            |
  +------------------------------------------------------------------------------------+     +--------------+     +------------+

  DIP1:                     DIP2:                     DIP3:
  +-------------------+     +-------------------+     +-------------------+
  | ON                |     | ON                |     | ON                |
  | +---------------+ |     | +---------------+ |     | +---------------+ |
  | |_|_|_|_|#|#|_|_| |     | |#|#|#|_|_|_|_|_| |     | |#|#|#|_|#|#|_|_| |
  | |#|#|#| | | |#|#| |     | | | | |#|#|#|#|#| |     | |#|#| |#| | |#|#| |
  | +---------------+ |     | +---------------+ |     | +---------------+ |
  |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |
  +-------------------+     +-------------------+     +-------------------+

  1x XTAL = 12 Mhz

  A = 74LS244
  B = 74LS161
  C = 74LS10
  D = 74LS11
  E = 74LS74
  F = 74LS273
  G = 74LS368
  H = 74LS32
  I = 74LS245
  J = 74166
  K = 74LS283
  L = 74LS138
  M = 74LS04
  N = 74LS139
  O = 74LS157
  P = 74LS04
  Q = NE555N
  R = 74HC00
  S = N82S123AN
  T = AM27S21APC
  U = 74LS02
  V = 74LS174
  W = 74LS08
  X = 74LS367
  Y = 74LS00
  Z = SN76489AN

*/
ROM_START( ns8linewa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.13b",   0x0000, 0x8000, CRC(c5692077) SHA1(423e0fe49ac450f22e693d9ac5ac1c3c662b17d3) )  // no match...

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.7h",  0x00000, 0x8000, CRC(25ca3657) SHA1(95ac8affbc309110159cd67e17d789c8bf426937) )  // no match...
	ROM_LOAD( "6.8h",  0x08000, 0x8000, CRC(80888d64) SHA1(91ec96709df77c534d381e391839984a88aeb1e0) )  // same as ns8lines
	ROM_LOAD( "7.9h",  0x10000, 0x8000, CRC(255d5860) SHA1(f171fde3d542594132b38b44300f750d45fb67a2) )  // same as ns8lines

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.1h",   0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )  // same as ns8lines
	ROM_LOAD( "2.2h",   0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )  // same as ns8lines
	ROM_LOAD( "3.4h",   0x4000, 0x2000, CRC(6be213c8) SHA1(bf5a002961b0827581cbab4249321ae5b51316f0) )  // same as ns8lines
	ROM_LOAD( "4.5h",   0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )  // same as ns8lines

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "2.13g", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )  // same as ns8lines
	ROM_LOAD( "1.14g", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )  // same as ns8lines

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "4.13d", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )  // same as ns8lines

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "3.3f",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )  // same as ns8lines

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "5.12d", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )  // same as ns8lines
ROM_END


/*
  New Lucky 8 Lines / New Super 8 Lines (W-4)

  Mini W4 board...

  Silkscreened: LUCKY97-1.
                06/07/1993.
                TAIWAN.

  GFX ROMs 1, 2, 3, 4 are double size. Each half matches the set ns8lines.
  GFX ROMs 5, 6, 7 match the ns8lines set.

  Program ROM is different.
*/
ROM_START( ns8linesa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8__27c256_main.14b",  0x0000, 0x8000, CRC(a3574e81) SHA1(60b037d2cfbad495897fa3e0fe6f6b81143103c6) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",  0x00000, 0x8000, CRC(994a9894) SHA1(4063c2c5e111f24a85df1665fd3f9fbb20fda4da) )
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(80888d64) SHA1(91ec96709df77c534d381e391839984a88aeb1e0) )
	ROM_LOAD( "7.bin",  0x10000, 0x8000, CRC(255d5860) SHA1(f171fde3d542594132b38b44300f750d45fb67a2) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1__27c128.1h",  0x0000, 0x2000, CRC(a1503f96) SHA1(908997a93bf7cdb3661b1a4ac7b1af3ad07982ef) )  // Double size. Identical halves. Each half matches the ns8lines set.
	ROM_IGNORE(                        0x2000)
	ROM_LOAD( "2__27c128.3h",  0x2000, 0x2000, CRC(bf6c534c) SHA1(3647e606eda53837100a5477b632d9655b24db45) )  // Double size. Identical halves. Each half matches the ns8lines set.
	ROM_IGNORE(                        0x2000)
	ROM_LOAD( "3__27c128.4h",  0x4000, 0x2000, CRC(a3ce1126) SHA1(13b840d569445149242aaa77758742d82d0f35ba) )  // Double size. Identical halves. Each half matches the ns8lines set.
	ROM_IGNORE(                        0x2000)
	ROM_LOAD( "4__27c128.5h",  0x6000, 0x2000, CRC(c1204efa) SHA1(6ed1991e75e5fec249d0a563e97888f63281e8d2) )  // Double size. Identical halves. Each half matches the ns8lines set.
	ROM_IGNORE(                        0x2000)

	ROM_REGION( 0x200, "proms", 0 )  // Taken from ns8lines. Seems to match 100%.
	ROM_LOAD( "u4.bin", 0x0000, 0x0100, BAD_DUMP CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "u5.bin", 0x0100, 0x0100, BAD_DUMP CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )  // Taken from ns8lines. Seems to match 100%.
	ROM_LOAD( "u2.bin", 0x0000, 0x0020, BAD_DUMP CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )  // Taken from ns8lines. Seems to match 100%.
	ROM_LOAD( "u3.bin", 0x0000, 0x0100, BAD_DUMP CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )  // Taken from ns8lines. Seems to match 100%.
	ROM_LOAD( "u1.bin", 0x0000, 0x0020, BAD_DUMP CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  Super 97-2

  Code jumps above $C000, so offset range C000-F7FF was defined
  as ROM space. However, the game isn't working due to the high
  memory code looks strange. Maybe the program needs some sort
  of address/data descramble...

*/
ROM_START( super972 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "subboard_27c512.bin",   0x0000, 0x10000, CRC(e5316735) SHA1(fc39a72e0146bb1190ccdeaec5eb59f779d0a3f7) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5_27c256.bin",  0x00000, 0x8000, CRC(19713d36) SHA1(ae04b8b72b0c9a279f24d7c4d619bac4629d9a4f) )
	ROM_LOAD( "6_27c256.bin",  0x08000, 0x8000, CRC(576197b9) SHA1(22273365cfe181f95efb895a28825f388b901a49) )
	ROM_LOAD( "7_27c256.bin",  0x10000, 0x8000, CRC(7a21f08b) SHA1(5795d06dcbbaee91e02dcc2e99451954ff45f768) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1_27c64.bin",  0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2_27c64.bin",  0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3_27c64.bin",  0x4000, 0x2000, CRC(b4e58020) SHA1(5c0fcc4b5d484ca7de5f2bd568a391a45967a9cc) )
	ROM_LOAD( "4_27c64.bin",  0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "dm74s287.g13", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "dm74s287.g14", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dm74s288.d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "dm74s287.f3",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "dm74s288.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  Mega Bonus Star II (Millennium Edition, Euro)
  Auto-Data Graz, 2002.

  W4 derivative hardware...

  PCB has a daughterboard with:
  - Z80.
  - Unknown DIP40 IC
  - M48T12 Timekeeper NVRAM.
  - HM6116L-70
  - GAL16V8D.
  - DS1232.
  - 74LS374N
  - SN74LS08N
  - 8 MHz Xtal.

  Here the DB layout...
  +-----------------------------+
  |        +-----------+ +----+ |
  |    +-+ |MBGEU_OR   | |U   | |
  |    | | |V.3.5.5    | |N   | |
  |+-+ |B| +-----------+ |K   | |
  || | | | +----+ +----+ |O   | |
  || | +-+ |HM61| |M48T| |W   | |
  ||A| +-+ |16L-| |12  | |N  Z| |
  || | | | |70  | |    | |   I| |
  || | |C| |    | |TIME| |   L| |
  || | | | |    | |KEEP| |   O| |
  |+-+ | | |    | |RAM | |   G| |
  |    +-+ +----+ +----+ +----+ |
  |    +----------------+       |
  |    |ZILOG           | XTAL  |
  |+-+ |Z0840006P       | 8.000 |
  ||D| +----------------+       |
  |+-+                          |
  +-----------------------------+

  A: GAL16V8D
  B: SN74LS08N
  C: DM74LS374N
  D: DS1232

  Unknown Zilog 40pin IC (scratched):
  (could be either a Z86C15 or Z86C91 MCU)

                    +-----V-----+
               VCC -|01       40|- 74LS08N (Pin 01)
        XTAL 8MHz <-|02       39|- GAL16V8 (Pin 17)
        XTAL 8MHz >-|03   Z   38|- 74LS374 (Pin 03)
               N/C -|04   I   37|- 74LS374 (Pin 04)
               N/C -|05   L   36|- 74LS374 (Pin 07)
  74LS08N (Pin 10) -|06   O   35|- 74LS374 (Pin 08)
  GAL16V8 (Pin 07) -|07   G   34|- 74LS374 (Pin 13)
  GAL16V8 (Pin 08) -|08       33|- 74LS374 (Pin 14)
  74LS374 (Pin 11) -|09       32|- 74LS374 (Pin 17)
               N/C -|10       31|- 74LS374 (Pin 18)
               GND -|11       30|- 74LS374 (Pin 01)
               N/C -|12       29|- 74LS08N (Pin 02)
      27C512 (A08) -|13       28|- 27C512 (Q7)
      27C512 (A09) -|14       27|- 27C512 (Q6)
      27C512 (A10) -|15       26|- 27C512 (Q5)
      27C512 (A11) -|16       25|- 27C512 (Q4)
      27C512 (A12) -|17       24|- 27C512 (Q3)
      27C512 (A13) -|18       23|- 27C512 (Q2)
      27C512 (A14) -|19       22|- 27C512 (Q1)
      27C512 (A15) -|20       21|- 27C512 (Q0)
                    +-----------+

  40-Pin DIP Pin Identification for Z86C15 MCU (for reference):

  ---------------------------------------------------------
  Pin    Symbol    Function                       Direction
  ---------------------------------------------------------
  01     VCC       Power Supply                   Input
  02     CLKOUT    Z8 System Clock                Output
  03     RCIN      RC Oscillator Clock            Input
  04     P37       Port 3, Pin 7                  Output
  05     P30       Port 3, Pin 0                  Input
  06     /RESET    Reset                          Input
  07     AGND      Analog Ground
  08     NC        Not Connected
  09     /WDTOUT   Watch-Dog Timer                Output
  10     P35       Port 3, Pin 5                  Output
  11     GND       Ground
  12     P32       Port 3, Pin 2                  Input
  13-20  P00-P07   Port 0, Pins 0,1,2,3,4,5,6,7   In/Output
  21-28  P10-P17   Port 1, Pins 0,1,2,3,4,5,6,7   In/Output
  29     P34       Port 3, Pin 4                  Output
  30     P33       Port 3, Pin 3                  Input
  31-38  P20-P27   Port 2, Pins 0,1,2,3,4,5,6,7   In/Output
  39     P31       Port 3, Pin 1                  Input
  40     P36       Port 3, Pin 6                  Output
  ---------------------------------------------------------

  BASE Board (based on WING W4):

  XTAL: 11.98135MHZ
  4x DIP-SWITCH

  1x YAMAHA YM2149F
  3x NEC D8255AC-2
  6x HY6116ALP-12

  Daughterboard is inserted in the Z80 Socket of the main board...

*/
ROM_START( mbs2euro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mbgeu_or_v.3.5.5.bin",   0x0000, 0x10000, CRC(b468a62f) SHA1(011536b08eb7cd42dd83826b195bbce314effda0) )

	ROM_REGION( 0x20000, "temp", 0 )
	ROM_LOAD( "mbs_21.bin",  0x00000, 0x8000, CRC(769f5793) SHA1(ffe542b8d1eee12738426b391a6cf61dbcc9fb3e) )  // GFX are in the last quarter.
	ROM_LOAD( "mbs_22.bin",  0x08000, 0x8000, CRC(2a4fa0f1) SHA1(1df7c7762aa6f4300c390a43092803cfd7ce46d3) )  // GFX are in the last quarter.
	ROM_LOAD( "mbs_23.bin",  0x10000, 0x8000, CRC(d47117ed) SHA1(ff5d981a70da7b08f04988e60624bad26529374f) )  // GFX are in the last quarter.
	ROM_LOAD( "mbs_24.bin",  0x18000, 0x8000, CRC(90fa917c) SHA1(d0de55c37c0bcc07586796189bb1e7a861f61a2d) )  // GFX are in the last quarter.

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "mbeu_5.bin",  0x00000, 0x8000, CRC(e97e90b4) SHA1(433b864d43b735dd043880e72eaabd0533530ceb) )
	ROM_LOAD( "mbeu_6.bin",  0x08000, 0x8000, CRC(9be871a7) SHA1(d4738be7207c121ab3b82bf01e19377b47956f56) )
	ROM_LOAD( "mbeu_7.bin",  0x10000, 0x8000, CRC(7a647742) SHA1(3f5433b85c81a94675fd681c18f2766d722a1f1f) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "temp",   0x06000, 0x0000, 0x2000 )
	ROM_COPY( "temp",   0x0e000, 0x2000, 0x2000 )
	ROM_COPY( "temp",   0x16000, 0x4000, 0x2000 )
	ROM_COPY( "temp",   0x1e000, 0x6000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "am27s21.g13", 0x0000, 0x0100, CRC(058b1195) SHA1(8e094e7a15d2ed7ff9d0336b0ea8a0a816e965e4) )
	ROM_LOAD( "am27s21.g14", 0x0100, 0x0100, CRC(0dcaa791) SHA1(69c68a22002b57d03b90e82b5a33d1df66c39362) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "82s123.d13",  0x0000, 0x0020, CRC(eacb8b76) SHA1(30cdd169a45b87c4262eea03ae28f910b091b100) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "am27s21.f3",  0x0000, 0x0100, CRC(169cbb68) SHA1(1062e84c4b4208be9aa400e236579dc5b83e9f83) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "82s123.d12",  0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )

	ROM_REGION( 0x800, "timekeeper", 0 )  // just for preservation and further analysis...
	ROM_LOAD( "m48t12.bin",  0x0000, 0x07f8, CRC(11e61f97) SHA1(57d4744747b9d75b3ac31da8fc621f1a00ff1de3) )  // shouldn't be 0x800?

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "gal16v8d.bin",  0x0000, 0x0892, CRC(1687b8da) SHA1(63c0bd119ec67c8033d08e03a6e6c86e0cab2884) )  // protected?... the first 0x800 are 1's
ROM_END


/*  Lucky Bar
    Unknown reels game based on Wing's Lucky 8 Lines.
    W-4 hardware

    Daughterboard with Z80 + program ROM + MC68705P3S MCU at location ic4 (14b)

    Game seems to be meant for progressive modes...
*/
ROM_START( luckybar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c256_subboard.ic4",   0x0000, 0x8000, CRC(b987115b) SHA1(1c2f6369170c9667996e5dde8ac93100a7234e19) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",  0x00000, 0x8000, CRC(994a9894) SHA1(4063c2c5e111f24a85df1665fd3f9fbb20fda4da) )
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(80888d64) SHA1(91ec96709df77c534d381e391839984a88aeb1e0) )
	ROM_LOAD( "7.bin",  0x10000, 0x8000, CRC(255d5860) SHA1(f171fde3d542594132b38b44300f750d45fb67a2) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(e3b96119) SHA1(292d2066e5deb442b9ba3403a73dbfac992c69c2) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(e355d816) SHA1(bf709fd0597dd28d5e8a020a54064749e3d29988) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(cf196c4a) SHA1(b36d9ab37590cbcf48352e6186bc137139ded0aa) )
	ROM_LOAD( "4.bin",   0x6000, 0x2000, CRC(1432a1c2) SHA1(c7cdf3501b9545b5463a7051d22765a9601ccad0) )

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "82s129.g13", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "82s129.g14", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "82s123.d13", 0x0000, 0x0020, CRC(c387bd86) SHA1(abf37c60e859dcedb3de2c85e7ed8e5545c9f73c) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "82s129.f3",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "82s123.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*  Cherry Angel
    (W-4 / CB3 hardware)

    Daughterboard with Z80 + program ROM + TTL at location ic4 (14b)

    Encryption pattern:
    For each 0x100 block:
    00-7f : 22 2a 82 2a...
    80-ff : a8 88 28 88...
*/
ROM_START( chryangla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_ca.sub",   0x0000, 0x10000, CRC(e0c9d088) SHA1(b96ae39616f595eb461242705689570f96467c8a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",  0x00000, 0x8000, CRC(5f4d51e4) SHA1(f88b786a3cb0c5b3609812d7cc5355a1791326b9) )
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(c078ac8a) SHA1(9befeacb20b4a1ff08cd273f54fdf7a849509fec) )
	ROM_LOAD( "7.bin",  0x10000, 0x8000, CRC(2311d258) SHA1(888a8c9d08bb7f0c35eacf02eae15f9ac0b816a8) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(32057eb7) SHA1(95379c43e476d7908186e39608548e7ef61dedc3) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "4.bin",   0x6000, 0x2000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x200, "proms", 0 ) /* proper dumps */
	ROM_LOAD( "82s129.g13", 0x0000, 0x0100, CRC(59ac98e4) SHA1(5fc0f1a48c49c956cdb8826e20663dc57a9175e4) )
	ROM_LOAD( "82s129.g14", 0x0100, 0x0100, CRC(0d8f35bd) SHA1(0c2a0145cdaaf9beabdce241731a36b0c65f18a2) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "82s123.d13", 0x0000, 0x0020, CRC(77a85e21) SHA1(3b41e0ab7cc55c5d78914d23e8289383f5bd5654) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "82s129.f3",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "82s123.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/******************************************************************************

  Lucky Lady (1985, Wing)
  -----------------------

  Wing original PCB

  Silkscreened: WING 8504-B - MADE IN JAPAN.

  1x 40-pin unknown encrypted CPU (numbered 03155096 and Falcon logo) (C11).
  1x AY-3-8910 (C5).
  3x NEC D8255AC-5 (C7-C8-C10).

  2x M5L27128K Program ROMs labeled 18 & 19 (B12-B13).
  7x GFX ROMs in a row labeled 11 to 17 (H1-H3-H4-H6-H7-H8-H10).

  1x Xtal @ 12MHz. (J6).
  4x 8 DIP switches banks: SW1(D8)-SW2(D7)-SW4(D6)-SW3(D5).
  1x 3.6 V. Lithium battery.
  1x 2x 10 pins edge connector.
  1x 2x 36 pins edge connector.
  1x simple 3-contacts switch.


  The following PROMs were added later:

  1x TBP18S030 @ D12 (32 bytes)
  1x TBP18S030 @ D13 (32 bytes)
  1x TBP24S10N @ F3 (256 bytes)
  1x TBP24S10N @ G13 (256 bytes)
  1x TBP24S10N @ G14 (256 bytes)


  RESNET: 2x resnet:

  @ D14:  1K - 470 - 220 - 1K - 470 - 220 - 470 - 220.
  @ F14:  1K - 470 - 220 - 1K - 1K - 470 - 220 - 1K - 470 - 220 - 1K.

******************************************************************************

  About encryption...

  Seems that programmers left a hole at offset 3890-3a90, enough to see some
  values and progressions as hints. Some of these strings (IE: 00 20 00 20),
  are valid to XOR some text offsets, as 3ac7-3acf, 3ae5-3aea, 3c00-3c0f, and
  other text strings. See also offsets 5240 and 5800 onward...


******************************************************************************/
ROM_START( luckylad )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* encrypted CPU */
	ROM_LOAD( "18.b12",  0x0000, 0x4000, CRC(2d178126) SHA1(5fc490e115e5c9073a7e3f56894fe19be6adb2b5) )
	ROM_LOAD( "19.b13",  0x4000, 0x4000, CRC(ad02b9fd) SHA1(1a85da2d418350e5cebdb889fa146565a72f37c4) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "15.h7",  0x00000, 0x8000, CRC(42f221eb) SHA1(24f764b176339b5ff49dc1836913724805f970d7) )
	ROM_LOAD( "16.h8",  0x08000, 0x8000, CRC(5af43f1e) SHA1(0f76071f1b43eab819bd892183730e9f7eed6c9b) )
	ROM_LOAD( "17.h10", 0x10000, 0x8000, CRC(096522e3) SHA1(900e448dd657088ecb1e6c72c3b0859efb2ec23f) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "11.h1",  0x0000, 0x2000, CRC(abe67459) SHA1(71223b99b79284b71ad6d2a1c8061ddd61e24e36) )
	ROM_LOAD( "12.h3",  0x2000, 0x2000, CRC(15d4bdae) SHA1(b2e4638bca29ec3ac662bc45f889a68c04cbd145) )
	ROM_LOAD( "13.h4",  0x4000, 0x2000, CRC(167303b4) SHA1(a0d4804d46854d832ac7047647e5f452bd0d87a2) )
	ROM_LOAD( "14.h5",  0x6000, 0x2000, CRC(708d5b2d) SHA1(a619b84bc67d579345db636d33e7e4f27e37a18c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "tbp24s10n.g13", 0x0000, 0x0100, CRC(b053242d) SHA1(00ed5c0d6694e83ae695fb0cdfed53be2b0bd6c9) )
	ROM_LOAD( "tbp24s10n.g14", 0x0100, 0x0100, CRC(46209d2a) SHA1(1d539219238ee47cdba3e7d44504619c7fbd2ade) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "tbp18s030.d13", 0x0000, 0x0020, CRC(41f3b9d3) SHA1(8b884a98e68ea438427227a64bf91fe86844a15f) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10n.f3", 0x0000, 0x0100, CRC(8de9cb92) SHA1(01449974fa27ebfd2583d6b2845c4f43938e082d) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "tbp18s030.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*
  Bingo, from Wing (1993).
  ------------------------

  Seems hardware derivated from Lucky 8 Lines.

  1x Z8400AB1  (11c)      Z80A CPU.
  3x D8255AC-2 (7c,8c,9c) Programmable Peripheral Interface.
  1x WF19054   (5c)       Programmable Sound Generator.
  1x oscillator (x1 6j)   12.000MHz Xtal.

  ROMs
  4x 2764 (1,2,3,4).
  4x 27256 (5,6,7,9).
  3x DM74S287N (3f,13g,14g)
  2x N82S123N  (12d,13d) not dumped yet.

  RAMs
  1x LC3517BSL-15 (14c)
  6x UM6116K-3 (1g,2g,3g,4g,11h,12)

  Others

  1x 36x2 edge connector.
  1x 10x2 edge connector.
  1x pushbutton.
  1x trimmer (volume).
  1x trimmer VAL.
  4x 8x2 DIP switches.
  1x battery 3.6V.

*/
ROM_START( bingowng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bingo9.14b", 0x0000, 0x8000, CRC(e041092e) SHA1(2aa3e7af08c336e49bed817ddad7c3604398e296) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "bingo5.7h",  0x00000, 0x8000, CRC(6e78690f) SHA1(140bbe502ff2deae4692a51b70704b9133a14cd4) )
	ROM_LOAD( "bingo6.8h",  0x08000, 0x8000, CRC(ab6897ce) SHA1(a4c1a6b7cd34b847b93a967ec88de4d22381944b) )
	ROM_LOAD( "bingo7.10h", 0x10000, 0x8000, CRC(f286b0a5) SHA1(b9b5e92f06c757a4f0c61078fe9a333c5f334920) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "bingo1.1h",  0x0000, 0x2000, CRC(a4382f0b) SHA1(01376c5355439be58acb743948d058b5aaaed051) )
	ROM_LOAD( "bingo2.3h",  0x2000, 0x2000, CRC(4650dd27) SHA1(90f858c60f66f897ab56d9ed7a0f7619a77efa73) )
	ROM_LOAD( "bingo3.4h",  0x4000, 0x2000, CRC(cea5a714) SHA1(7fb40588823fed0737f00a2597405bb5d2662406) )
	ROM_LOAD( "bingo4.5h",  0x6000, 0x2000, CRC(efc57761) SHA1(211b5f5587ec43b3769646fe17e5af2c0136f300) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "dm74s287n.13g", 0x0000, 0x0100, CRC(95e3c686) SHA1(33e1f33209b4c9e25619233810dd7b2e8d217e5a) )
	ROM_LOAD( "dm74s287n.14g", 0x0100, 0x0100, CRC(452473ad) SHA1(2facfa7ebe2f20c4b172991cce9b6faad98ef4a2) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s123n.13d", 0x0000, 0x0020, BAD_DUMP CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) ) // taken from other set

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "dm74s287n.3f", 0x0000, 0x0100, CRC(cc8202f5) SHA1(91411d3d828bc84323164c64f481d05210efda97) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "n82s123n.12d", 0x0000, 0x0020, BAD_DUMP CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) ) // taken from other set
ROM_END


ROM_START( bingownga )  /* This set is coming from Dumping Union */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bingo.14b",  0x0000, 0x8000, CRC(e041092e) SHA1(2aa3e7af08c336e49bed817ddad7c3604398e296) )  // identical halves, same original program

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "bingo-5.h7",  0x00000, 0x8000, CRC(6b875fe5) SHA1(a2a214942584a2db2a17047264f4d62d0a746906) )
	ROM_LOAD( "bingo-6.h8",  0x08000, 0x8000, CRC(2f156566) SHA1(7eccbc452550087f40fed1de5fa30a3290fb3c07) )
	ROM_LOAD( "bingo-7.h10", 0x10000, 0x8000, CRC(cd0446ef) SHA1(42c15781a1d14f29b7f51936a179a1c7a183d03b) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "bingo-1.h1", 0x00000, 0x8000, CRC(fcfc888d) SHA1(13b73ad198c2bc12a0e6e146e28471d77d1d52de) )
	ROM_LOAD( "bingo-2.h3", 0x08000, 0x8000, CRC(60e9065b) SHA1(d40fb01bf90b7f4fcc1da9cbe70efd04e7c5a220) )
	ROM_LOAD( "bingo-3.h4", 0x10000, 0x8000, CRC(d7ecece9) SHA1(f5b551fac7f326f14d5ee191e930380bdce555c6) )
	ROM_LOAD( "bingo-4.h6", 0x18000, 0x8000, CRC(16c840f8) SHA1(5612f09ee8bf69b6da1e52295d30179108926f86) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "dm74s287n.13g", 0x0000, 0x0100, CRC(95e3c686) SHA1(33e1f33209b4c9e25619233810dd7b2e8d217e5a) )
	ROM_LOAD( "dm74s287n.14g", 0x0100, 0x0100, CRC(452473ad) SHA1(2facfa7ebe2f20c4b172991cce9b6faad98ef4a2) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s123n.13d", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "dm74s287n.3f", 0x0000, 0x0100, CRC(cc8202f5) SHA1(91411d3d828bc84323164c64f481d05210efda97) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "n82s123n.12d", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*

Magical Tonic

unknown, 40 pin cpu (plastic box, with "Tonic" sticker on it)
8255 x3
YM2203
12 MHz

4x DSW

is this the original Magical Odds?
*/
void wingco_state::init_magoddsc()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int A = 0; A < 0x8000; A++)
	{
		if ((A & 4) == 4)
			ROM[A] ^= 0x01;

		ROM[A] = bitswap<8>(ROM[A], 3,6,5,4,7,2,1,0);
	}
}


// is this a bootleg board?
ROM_START( magodds )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "8_p6_d12.512", 0x00000, 0x08000, CRC(6978c662) SHA1(cfdbcdcd4085c264e1d0ad4f18160b40d2d4e406) )
	ROM_IGNORE(0x8000) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "9_p7_d14.512", 0x0c000, 0x04000, CRC(095230ba) SHA1(e60f5497c2cd5f1c0fc33b1e21303dd569654e6d) )
	ROM_IGNORE(0xc000) // BADADDR        --magoddscxxxxxx

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5_l5_g2.512", 0x20000, 0x10000, CRC(f0ffb199) SHA1(383406e6ab74e8cecaf5574a879bf24aa606ac37) )
	ROM_LOAD( "6_l6_g3.512", 0x10000, 0x10000, CRC(3e44d92b) SHA1(4e00e26a4dbf326c0c919c40382505189e82c85b) )
	ROM_LOAD( "7_l7_g5.512", 0x00000, 0x10000, CRC(654bb754) SHA1(346bfbf85fc38797cf422da47d474e2ef2ef459c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1_m1_h2.764", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "2_m2_h3.764", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "3_m3_h5.764", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "4_m4_h6.764", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END


// is this a bootleg board?
// program is the same as above set (but without the oversized rom 9), only gfx1 differs
// the proms came from this board
ROM_START( magoddsa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "8.12e", 0x00000, 0x08000, CRC(6978c662) SHA1(cfdbcdcd4085c264e1d0ad4f18160b40d2d4e406) )
	ROM_IGNORE(0x8000) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "9.14e", 0x0c000, 0x04000, CRC(b3661c55) SHA1(f576d434ccec6f342455c18ada156d29634627cb) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.2f", 0x20000, 0x10000, CRC(39e6ad6f) SHA1(0075015b3e794d82fb62984a41be48d17833c9f0) )
	ROM_LOAD( "6.3f", 0x10000, 0x10000, CRC(42e03002) SHA1(5c4f2a1aa91d3c0906665bec7423f8579f434dc1) )
	ROM_LOAD( "7.5f", 0x00000, 0x10000, CRC(5301a9a4) SHA1(f592bfc5f0d835cab6029aed7d6dcdd9d2ee1a2c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1_m1_h2.764", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "2_m2_h3.764", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "3_m3_h5.764", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "4_m4_h6.764", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END


// gfx same as above set, program rom differs
ROM_START( magoddsb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mo-137-8.p6", 0x00000, 0x08000, CRC(abe5d6d1) SHA1(45161c580e6852aae5d9dbc06bb9743e86f9c279) )
	ROM_LOAD( "mo-137-9.p7", 0x0c000, 0x04000, CRC(b3661c55) SHA1(f576d434ccec6f342455c18ada156d29634627cb) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "mo-137-5.l5", 0x20000, 0x10000, CRC(39e6ad6f) SHA1(0075015b3e794d82fb62984a41be48d17833c9f0) )
	ROM_LOAD( "mo-137-6.l6", 0x10000, 0x10000, CRC(42e03002) SHA1(5c4f2a1aa91d3c0906665bec7423f8579f434dc1) )
	ROM_LOAD( "mo-137-7.l7", 0x00000, 0x10000, CRC(5301a9a4) SHA1(f592bfc5f0d835cab6029aed7d6dcdd9d2ee1a2c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "mo-137-1.m1", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "mo-137-2.m2", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "mo-137-3.m3", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "mo-137-4.m4", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n5.bin", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "n7.bin", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "n6.bin", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n4.bin", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "n9.bin", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "n8.bin", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "p1.bin", 0x40, 0x20, CRC(1aa176f3) SHA1(fe777cba829046f850ab612b927bde4fe0d37811) ) // different! to magoddsa
ROM_END


// custom CPU block
ROM_START( magoddsc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.e6",   0x0000, 0x8000, CRC(01daf2af) SHA1(cb9b12c79dce3c9123510a49dffc9f3cee056cf6) )
	ROM_LOAD( "9.e6",   0x8000, 0x8000, CRC(1770ac79) SHA1(cadfd00ae75b90b1d202d741828e0afbd5ba0bec) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.j10",  0x00000, 0x10000, CRC(b8032ea3) SHA1(15e5335a583d8e7a5422cd4d1d7342874a4962ab) )
	ROM_LOAD( "6.j11",  0x10000, 0x10000, CRC(ff38ff30) SHA1(8fef6e1fe7c307c69c9dcafa69ecf66467b9cb41) )
	ROM_LOAD( "7.j12",  0x20000, 0x10000, CRC(8f1d2db9) SHA1(200de01334905079dca542541e442d4194ecd913) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "1.l10",  0x0000, 0x4000, CRC(746588db) SHA1(2a0af552011246d4cc0cd0b670907cf8685ce8ef) )
	ROM_LOAD( "2.l11",  0x4000, 0x4000, CRC(8b7dd248) SHA1(a3ebde9fd0b6b1e42aa9b6d8e30c225abf2f80ce) )
	ROM_LOAD( "3.l12",  0x8000, 0x4000, CRC(de05e678) SHA1(8b9fcb9f912075a20a9ae38100006b57d508e0e7) )
	ROM_LOAD( "4.l13",  0xc000, 0x4000, CRC(8c542eee) SHA1(cb424e2a67c6d39302beca7cd5244bcad4a91189) )

	// proms not verified on this set, taken from magoddsa
	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END


// custom CPU block
ROM_START( magoddsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.p7",   0x0000, 0x8000, CRC(da0f1ae5) SHA1(5790c8ec6fbcd13088079c1fbd035ef816b423e6) )
	ROM_LOAD( "9.p6", 0x0c000, 0x04000, CRC(b3661c55) SHA1(f576d434ccec6f342455c18ada156d29634627cb) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.m2", 0x20000, 0x10000, CRC(39e6ad6f) SHA1(0075015b3e794d82fb62984a41be48d17833c9f0) )
	ROM_LOAD( "6.m3", 0x10000, 0x10000, CRC(42e03002) SHA1(5c4f2a1aa91d3c0906665bec7423f8579f434dc1) )
	ROM_LOAD( "7.m4", 0x00000, 0x10000, CRC(5301a9a4) SHA1(f592bfc5f0d835cab6029aed7d6dcdd9d2ee1a2c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.m1", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "2.m2", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "3.m3", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "4.m4", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	// proms not verified on this set, taken from magoddsa
	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END


/*
    LADY LINER - TAB Austria

    Hardware Notes:
    ---------------

    CPU:   1x Z80.
    Sound: 1x AY8930.
    I/O:   2x P8255A.

    Clock: 1x Xtal @ 12.0000MHz.

    ROMs:  1x NM27C256Q (ladybrd)
           7x M27C64A (1,2,3,4,71,72,73)
           2x PROM AM27S19PC (39,73)
           3x PROM AM27S21PC (37,38,96)

    RAM:   4x HY6116ALP-10 (near graphics ROMs)
           1x HY6116ALP-10 (near program ROM)

    1x 8 DIP Switches.
    1x trimmer (volume).

    Connectors:  1x 18x2 edge connector.
                 1x 22x2 edge connector.

    Both connectors are wired to a strange small PCB with:
    1x 6x2 edge connector + 1x 60x2 edge connector (with smaller spacing) + 2x 8 DIP Switches.

    Silkscreened on PCB:
    "TAB AUSTRIA"

    Sticker on PCB:
    "TAB Austria" & "LL 2690"

*/
ROM_START( ladylinr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ladybrd.bin",    0x0000, 0x8000, CRC(44d2aed0) SHA1(1afe6178d1bf4ad0b623f33be879ed5180ad2db1) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ll73.bin",   0x00000, 0x8000, CRC(afa4a705) SHA1(779340713df7029553cfc1c57997dfdd96a0f0cc) )
	ROM_LOAD( "ll72.bin",   0x08000, 0x8000, CRC(bd1d8a39) SHA1(01e37704c753352024e79b0b83b040f8288b9aed) )
	ROM_LOAD( "ll71.bin",   0x10000, 0x8000, CRC(1c417efa) SHA1(491579a76d80c4f488ef94393d12a190571ae285) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2.bin",  0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3.bin",  0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4.bin",  0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s21pc.38",   0x0000, 0x0100, CRC(06a0ee6f) SHA1(e793fbb9e14e4e6c6d6783a36edee74f28e7e214) )
	ROM_LOAD( "am27s21pc.37",   0x0100, 0x0100, CRC(8589d23c) SHA1(9629c0d8af3cce47ef376898a4be84c0752a265b) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "am27s19pc.39",   0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "am27s21pc.96",   0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "am27s19pc.73",   0x0000, 0x0020, CRC(b48d0b41) SHA1(01d2d0fd5e79c17043e97146001150b4b32ac86c) )
ROM_END


/*
  Board had a sticker that said MODEL 9006

  .u22  2764
  .u21  2764
  .u20  2764
  .u19  2764
  .u18  27256
  .u17  27256
  .u16  27256
  .u66  27256

  HM6116P
  D4016C-2 x3
  D4016C-5
  12Mhz Crystal
  D780C
  UM82C55A-PC
  M5L8255AP-5
  D8255AC-2

*/
ROM_START( kkotnoli )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "9006.u66",   0x0000, 0x8000, CRC(5807a005) SHA1(9c7156656cd651c7785c42ce25e96aadd8e3d9ff) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "9006.u18",   0x00000, 0x8000, CRC(b83e304f) SHA1(1022518928d8cb094340927a65b8fb72b0f3c640) )
	ROM_LOAD( "9006.u17",   0x08000, 0x8000, CRC(13a3258b) SHA1(605555ae48012ca4b1829d1c835b33ddeec17da6) )
	ROM_LOAD( "9006.u16",   0x10000, 0x8000, CRC(84d09be4) SHA1(09a9e491a9a5fc7882c88d46ae2a6e7e99d082f4) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* redumped */
	ROM_LOAD( "9006.u22",   0x0000, 0x2000, CRC(4e93130d) SHA1(faaaf51844da8d3bdb908fb8ce0f2442e26b5f62) )
	ROM_LOAD( "9006.u20",   0x2000, 0x2000, CRC(717fe736) SHA1(04e578c1992bbdb312bb6bc12137bd96522a50e6) )
	ROM_LOAD( "9006.u21",   0x4000, 0x2000, CRC(f5314f3f) SHA1(0423dc545fce0322377f1934894a999427709b33) )
	ROM_LOAD( "9006.u19",   0x6000, 0x2000, CRC(c321d50b) SHA1(8c132d8fcc812bcec5966c8a3960dfbe5d9f8c36) )

	/* proper proms recent dumped */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "9006.u24", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "9006.u23", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	/* this one seems bitrotten (bits 3 and 7), except for this issue,
	   the prom have concordance with prom1 from lucky8 */
	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom1", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )
//  ROM_LOAD( "9006.u57", 0x0000, 0x0020, CRC(8a37416a) SHA1(696b46db2ff2bb9ef471ff925977e8a186b17de8) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "9006.u41", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "9006.u58", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*

Wild cat 3 by E.A.I.

Cherry master type game

.h1  2764   handwritten 1
.h3  2764   handwritten 2
.h4  2764   handwritten 3
.h5  2764   handwritten 4
.h7  27256  handwritten 5
.h8  27256  handwritten 6
.h10 27256  stickered 7 E.A.I.
.g13 82s129 stickered G13
.g14 82s129 stickered G14
.d12 82s123 handwritten 2
.d13 82s123 stickered D13
.f3  82s129 handwritten 3

open 24 pin socket @ B13

Daughter board
.u5  27512

Z80 on daughter board
SN76489AN
6116 x4
12.000 MHz crystal
8255 x3
Winbound WF19054 40 pin dip

*/
ROM_START( wcat3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wcat3.u5",   0x0000, 0x10000, CRC(bf21cde5) SHA1(b501ba8ea815e3b19b26196f6fd48243892278eb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "wcat3.h7",   0x10000, 0x8000, CRC(065cb575) SHA1(4dd49773c4caeaa489342e61f26c8eaaae876edc) )
	ROM_LOAD( "wcat3.h8",   0x08000, 0x8000, CRC(60463213) SHA1(b0937b4a55f74831ce9a06f3df0af504845f908d) )
	ROM_LOAD( "wcat3.h10",  0x00000, 0x8000, CRC(dda38c26) SHA1(4b9292911133dd6067a1c61a44845e824e88a52d) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "wcat3.h1",   0x6000, 0x2000, CRC(0509d556) SHA1(c2f46d279f45b544c67b0c966659cc6d5d53c22f) )
	ROM_LOAD( "wcat3.h2",   0x4000, 0x2000, CRC(d50f3d62) SHA1(8500c7f3a2f51ea0ed7e142ecdc4e669ba3e7065) )
	ROM_LOAD( "wcat3.h4",   0x2000, 0x2000, CRC(373d9949) SHA1(ff483505fb9e86411acad7059bf5434dde290946) )
	ROM_LOAD( "wcat3.h5",   0x0000, 0x2000, CRC(50febe3b) SHA1(0479bcee53b174aa0413951e283e446b09a6f156) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "wcat3.g13",  0x0000, 0x0100, CRC(c29a36f2) SHA1(936b07a195f6e7f6a884bd35f442003cf67aa447) )
	ROM_LOAD( "wcat3.g14",  0x0100, 0x0100, CRC(dcd53d2c) SHA1(bbcb4266117c3cd1c8ef0e5046d3558c8293313a) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "wcat3.d12",  0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "wcat3.f3",   0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "wcat3.d13",  0x0000, 0x0020, CRC(eab832ed) SHA1(0fbc8914ba1805cfc6698fe7f137a934e63a4f89) )
ROM_END


/* these 'Amcoe' games look like bootlegs of cherry master
  the z80 roms are encrypted */
ROM_START( skill98 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sk98133.bin", 0x0000, 0x1000, CRC(77a5dd54) SHA1(e693f477b42b83f1f5e45fb7c56486119bf91856) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sk98h.bin",  0x00000, 0x10000, CRC(0574357b) SHA1(96a846f6d49dd67ad078ad9240e632f79ae1b437) )
	ROM_LOAD( "sk98l.bin",  0x10000, 0x10000, CRC(ebe802a4) SHA1(178542c204fd1027874e6d2c099edaa7878c993f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sk98u19.bin", 0x0000, 0x0100, CRC(796c7960) SHA1(0f64a8119fd4809a5ac79585b415b34b2a83e9dc) )
	ROM_LOAD( "sk98u20.bin", 0x0100, 0x0100, CRC(a0862663) SHA1(e27c3bba5f87b51a19ea8068f2ce7b82a6f0eedb) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "sku1920.bin", 0x0000, 0x0100, CRC(a8c86d5e) SHA1(d19cd5e57ac8fdd685540c1bb2e1474d1326362b) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sk98t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END


ROM_START( schery97 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sc352.bin", 0x00000, 0x1000, CRC(d3857d85) SHA1(e97b2634f0993631023c08f6baf800461abfad12) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sc97h.bin",  0x00000, 0x10000, CRC(def39ee2) SHA1(5e6817bd947ebf16d0313285a00876b796b71cab) )
	ROM_LOAD( "sc97l.bin",  0x10000, 0x10000, CRC(6f4d6aea) SHA1(6809c26e6975cac97b0f8c01a508d4e022859b1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sc97u19.bin", 0x0000, 0x0100, CRC(6a01caca) SHA1(9b3e9eebb9fcc8770f7e92f0f1c0434516ee613d) )
	ROM_LOAD( "sc97u20.bin", 0x0100, 0x0100, CRC(5899c1d5) SHA1(c335b99bb58da3a11005a8952a15d9f43bdff157) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "scu1920.bin", 0x0000, 0x0100, CRC(3aa291dd) SHA1(f35c916b5463ff9ec6e57048af29a746148a13af) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sc97t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END


ROM_START( schery97a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sc352c4.bin", 0x00000, 0x1000, CRC(44f55f6e) SHA1(8b6e8618281de480979de37c7b36a0e68a524f47) ) // ?? alt program?
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sc97h.bin",  0x00000, 0x10000, CRC(def39ee2) SHA1(5e6817bd947ebf16d0313285a00876b796b71cab) )
	ROM_LOAD( "sc97l.bin",  0x10000, 0x10000, CRC(6f4d6aea) SHA1(6809c26e6975cac97b0f8c01a508d4e022859b1a) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sc97u19.bin", 0x0000, 0x0100, CRC(6a01caca) SHA1(9b3e9eebb9fcc8770f7e92f0f1c0434516ee613d) )
	ROM_LOAD( "sc97u20.bin", 0x0100, 0x0100, CRC(5899c1d5) SHA1(c335b99bb58da3a11005a8952a15d9f43bdff157) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "scu1920.bin", 0x0000, 0x0100, CRC(3aa291dd) SHA1(f35c916b5463ff9ec6e57048af29a746148a13af) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sc97t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END


ROM_START( roypok96 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp35.bin", 0x0000, 0x1000, CRC(e1509440) SHA1(30d931b02d4eb74f9a16c57eb12e834cf24f87a9) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END


ROM_START( roypok96a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp36.bin", 0x0000, 0x1000, CRC(7fffff21) SHA1(85533e6aa0c6810cdaed9a6d1f1313f7bc871cbd) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)


	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END


ROM_START( roypok96b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp36c3.bin", 0x0000, 0x1000, CRC(c7317ed7) SHA1(ca88d02c5ea5c03dd9407d71ab88e81c21791fe8) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END


ROM_START( roypok96c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "subboard.bin", 0x0000, 0x1000, CRC(5ce4bed1) SHA1(bd888fec4531157668d4b14a9439a3051e52b3ec) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END


ROM_START( pokonl97 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "po33.bin", 0x00000, 0x1000, CRC(55bdd5cf) SHA1(7fd9e5c63ab2439db33710d7684f5df5e7324325) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "po97h.bin",  0x00000, 0x10000, CRC(fe845426) SHA1(80a1ffa28f92ad381ccf01b387afddd3ee849a58) )
	ROM_LOAD( "po97l.bin",  0x10000, 0x10000, CRC(d389d5be) SHA1(a88db3bf411dd1bdf8dc42c8c440d71b24ef95ee) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "po97u19.bin", 0x0000, 0x0100, CRC(889dd4b3) SHA1(dc4b2506bf61f1bc4d491c3a9c410be11d93b76f) )
	ROM_LOAD( "po97u20.bin", 0x0100, 0x0100, CRC(e44d1b48) SHA1(0a21b79c03f33d31303ba6cabc4b5a23d7c9cfe3) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "pou1920.bin", 0x0000, 0x0100, CRC(ceac07bb) SHA1(b6fca4ef937c0a75d6371db405faf15d69462fc4) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "po97t.bin", 0x00000, 0x20000, CRC(dab7cbeb) SHA1(40cf5717485f31d5b5267a9f79ead0d21509d68c) )
ROM_END


ROM_START( nfb96 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fb363c1.bin", 0x00000, 0x1000, CRC(ddc22974) SHA1(86f5d4d32f69d054ee1b444e2c4d470603e0391c) ) // v3.63, C1 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fbseh.bin",  0x00000, 0x10000, CRC(2fc10ce7) SHA1(a2418cfbe7ed217848ace8ea06587bcaa6b2c8f2) )
	ROM_LOAD( "fbsel.bin",  0x10000, 0x10000, CRC(fb9d679a) SHA1(a4f6246bdbbf2e25f702006b30a62bc7873137de) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nfb96a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fb362c1.bin", 0x00000, 0x1000, CRC(5054418a) SHA1(a4bfe05b0eb3476651c06cb9ff78051e55c943c5) ) // v3.62, C1 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fbseh.bin",  0x00000, 0x10000, CRC(2fc10ce7) SHA1(a2418cfbe7ed217848ace8ea06587bcaa6b2c8f2) )
	ROM_LOAD( "fbsel.bin",  0x10000, 0x10000, CRC(fb9d679a) SHA1(a4f6246bdbbf2e25f702006b30a62bc7873137de) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nfb96b )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fbse354d.bin", 0x00000, 0x1000, CRC(89dd3290) SHA1(040aa1daae58a38db66a046a4379975e014a2598) ) // v3.54, D Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fb96seh.bin",  0x00000, 0x10000, CRC(12042a0a) SHA1(4bc5f87f4b92f303fef100bf16e3d7b27670b793) )
	ROM_LOAD( "fb96sel.bin",  0x10000, 0x10000, CRC(d611f10b) SHA1(425cad584e85f21de214bf978555a7811b13aa35) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nfb96c )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fbse362dk.bin", 0x00000, 0x1000, CRC(aa2576f2) SHA1(b6f0d6078dee01b90e08c09008f299439768c266) ) // v3.62,DK Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fb96seh.bin",  0x00000, 0x10000, CRC(12042a0a) SHA1(4bc5f87f4b92f303fef100bf16e3d7b27670b793) )
	ROM_LOAD( "fb96sel.bin",  0x10000, 0x10000, CRC(d611f10b) SHA1(425cad584e85f21de214bf978555a7811b13aa35) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nfb96txt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tf122axt.bin", 0x00000, 0x1000, CRC(50b5b105) SHA1(01348c463985d9967b2494b649fa02edbd61f698) ) // Special Texas v1.22, C2 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "txfbh.bin",  0x00000, 0x10000, CRC(9e9ba897) SHA1(901bb2596ff67c0290977fd508247aa1da0a09b9) )
	ROM_LOAD( "txfbl.bin",  0x10000, 0x10000, CRC(d1b8920c) SHA1(c6dc065134724baafed4c1cfa4aaf3c23dfb7a32) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ch363c1.prom", 0x00000, 0x1000, CRC(e566cea3) SHA1(be81b21267ae2ff64c4dbc58e99a9ff4bf1d21aa) ) // v3.63, C1 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96seh.bin",  0x00000, 0x10000, CRC(65dee6ba) SHA1(77f5769ed0b745a4735576e9f0ce90dcdd9b5410) ) /* Correct graphics for this set, shows (R)COPYRIGHT(C) 1996 */
	ROM_LOAD( "ch96sel.bin",  0x10000, 0x10000, CRC(c21cc114) SHA1(f7b6ff5ac34dc1a7332e8c1b9cc40f3b65deac05) ) /* the other set shows a currupted TM graphic */

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ch362c1.prom", 0x00000, 0x1000, CRC(10fa984b) SHA1(92ed9838db28deca038c4a5c080a723075ae0c53) ) // v3.62, C1 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96se-h.vrom2",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "ch96se-l.vrom1",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96b )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ch361c1.prom", 0x00000, 0x1000, CRC(dfdd3869) SHA1(e512dd9370b40285a35d12ba7a05e65ab1c693ab) ) // v3.61, C1 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96se-h.vrom2",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "ch96se-l.vrom1",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96c )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chse354d.bin", 0x00000, 0x1000, CRC(160f7b78) SHA1(537a91317e613676b748d4e4ec7015183872814b) ) // v3.54, D Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96seh.bin",  0x00000, 0x10000, CRC(65dee6ba) SHA1(77f5769ed0b745a4735576e9f0ce90dcdd9b5410) ) /* Correct graphics for this set, shows TM COPYRIGHT(C) 1996 */
	ROM_LOAD( "ch96sel.bin",  0x10000, 0x10000, CRC(c21cc114) SHA1(f7b6ff5ac34dc1a7332e8c1b9cc40f3b65deac05) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96d )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chse353d.prom", 0x00000, 0x1000, CRC(fc56f84f) SHA1(a98746ef0dc919ced201c98c835ef0cbd5c18627) ) // v3.54, D Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96se-h.vrom2",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "ch96se-l.vrom1",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96e )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chsesep3_3.40.prom", 0x00000, 0x1000, CRC(0eec1fd1) SHA1(93e096f471376f5efc3b229fd226c48cd623c48d) ) // v3.40, D Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96se-h.vrom2",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "ch96se-l.vrom1",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96f )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chse362dk.bin", 0x00000, 0x1000, CRC(66df35d3) SHA1(da33f6413b8cf7b472023abc3e5cfe6d52ed1418) ) // v3.62, DK Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96seh.bin",  0x00000, 0x10000, CRC(65dee6ba) SHA1(77f5769ed0b745a4735576e9f0ce90dcdd9b5410) )
	ROM_LOAD( "ch96sel.bin",  0x10000, 0x10000, CRC(c21cc114) SHA1(f7b6ff5ac34dc1a7332e8c1b9cc40f3b65deac05) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "82s129a.u84", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "82s129a.u79", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nc96txt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tc132axt.bin", 0x00000, 0x1000, CRC(a77dc042) SHA1(1076a6687da1871b666bd214a755b68f5e1aeb8c) ) // Special Texas v1.32, C2 Sub-PCB
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "txchh.bin",  0x00000, 0x10000, CRC(07621bde) SHA1(65fd52545a399694394e623a7249d180d1a4fa9f) )
	ROM_LOAD( "txchl.bin",  0x10000, 0x10000, CRC(3b00b7dc) SHA1(99d513d23eea28fedf59ad272a3280abd0d3c2ab) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( match98 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "match133.bin", 0x00000, 0x1000, CRC(ddd82435) SHA1(4d7310f77e1f87e2b5c820a311aaefd82307b388) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "match98h.bin",  0x00000, 0x10000, CRC(94899f26) SHA1(1b6f953b6251496d7d06fb0a2d0b861e04ebc3df) )
	ROM_LOAD( "match98l.bin",  0x10000, 0x10000, CRC(6db4b962) SHA1(7d476e244d70a86dacf85dd9c790f63aef4b7cd9) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "matchu19.bin", 0x0000, 0x0100, CRC(59cd3566) SHA1(e9726aad631d86e2c122e3d75f87abc22ea7ef97) )
	ROM_LOAD( "matchu20.bin", 0x0100, 0x0100, CRC(f421503c) SHA1(591c80d6ff63021fab31b3bfcde1b47cd75fd7bb) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "matu1920.bin", 0x0000, 0x0100, CRC(c249576f) SHA1(54d51a54f4b2503c706c1c06050e33be7f479dfc) )

	ROM_REGION( 0x100, "proms3", 0 ) // ? none of the other sets have this
	ROM_LOAD( "matchu8.bin", 0x0000, 0x0100, CRC(dba4579d) SHA1(fba0a5adad13728c805fbe9666a8e02484cfa821) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	ROM_LOAD( "match98t.bin", 0x00000, 0x40000, CRC(830f4e01) SHA1(fbc41e9100a69663b0f799aee447edd5fabd2af7) )
ROM_END


ROM_START( fb2010 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fb2013r.bin", 0x00000, 0x1000, CRC(9cc75315) SHA1(f77fbce1037dbf38ddaa4ce79266d62e5cc7989e) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "high.bin", 0x00000, 0x10000, CRC(5950b5fb) SHA1(64441fdbd768e7765e20a33acd4002e69b868f09) )
	ROM_LOAD( "low.bin",  0x10000, 0x10000, CRC(98b0454f) SHA1(91f7f4119a0cd591e68c87a9e716a8cd5233a4aa) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette (taken from nfb96se, not verified)
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, BAD_DUMP CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, BAD_DUMP CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )
ROM_END


void cmaster_state::init_fb2010()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0x22)
		{
			case 0x00: x = bitswap<8>(x^0x4c^0xff, 0,4,7,6,5,1,3,2); break;
			case 0x02: x = bitswap<8>(x^0xc0^0xff, 7,6,0,5,3,2,1,4); break;
			case 0x20: x = bitswap<8>(x^0x6b^0xff, 4,3,2,7,5,6,0,1); break;
			case 0x22: x = bitswap<8>(x^0x23^0xff, 0,6,1,3,4,5,2,7); break;
		}

		ROM[i] = x;
	}

	// some kind of protection? checks something in NVRAM after a few spins?
	// TODO: work out how to handle this without a patch, doesn't seem 100% related to port read below like in other games?
	ROM[0x10dc] = 0x00;
	ROM[0x10dd] = 0x00;

	m_maincpu->space(AS_IO).install_read_handler(0x1e, 0x1e, read8_delegate(FUNC(cmaster_state::fixedval7d_r),this));
}


/* descrambled by looking at CALLs

0000 -> 0000

46e7 -> 16e7
4027 -> 1027

35f3 -> 25f3
3327 -> 2327

7f6a -> 3f6a

1095 -> 4095
1d2f -> 4d2f
1e8b -> 4e8b

6246 -> 5246
628f -> 528f

2bed -> 6bed
2db7 -> 6db7

5838 -> 7838
58a2 -> 78a2

810f -> 810f

9762 -> 9762

a??? -> a???

b84a -> b84a

c??? -> c???

*/
ROM_START( nfb96se )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogdptb.prg",0x00000, 0x1000, CRC(0690f915) SHA1(ed2477ba260a421013603017cfd1e1ba5ecd7f4e) ) // alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogd5.rom",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dogd6.rom",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dogd7.rom",   0x00000, 0x08000, CRC(2f03f1e2) SHA1(b221ad7177fcf4d6d65b8ee9c0f5e4289688c707) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogd1.rom",  0x6000, 0x2000, CRC(0f9f77b1) SHA1(03719f79a39f93f38e4170143a5654bd74596206) )
	ROM_LOAD( "dogd2.rom",  0x4000, 0x2000, CRC(6ab19916) SHA1(f125365b3c5546d72662cf439311811ae761f225) )
	ROM_LOAD( "dogd3.rom",  0x2000, 0x2000, CRC(5d4810a5) SHA1(8e9e50c6c7c13010ecb726041a1ac8eccead96ce) )
	ROM_LOAD( "dogd4.rom",  0x0000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END


// this set has an encrypted program rom
ROM_START( nfb96sea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dog_08.rom",   0x00000, 0x1000, CRC(357f13e8) SHA1(ca0872c9f7dc44a4c1c342f7f53c490f6342f1d2) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	/* the graphic roms on this set are a mess, the planes don't match up properly */
	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dog_05.rom",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dog_06.rom",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dog_07.rom",   0x00000, 0x08000, CRC(97033c70) SHA1(4d5746f43f8f4d374ba2f31d21defd21921d39bf) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dog_01.rom", 0x0000, 0x2000, CRC(26eb35ce) SHA1(564c775eda7a026841095b210257aea59288f042) )
	ROM_LOAD( "dog_02.rom", 0x2000, 0x2000, CRC(0e220d8a) SHA1(f84145250785dae78ea5af6388d91ad24b42ff9c) )
	ROM_LOAD( "dog_03.rom", 0x4000, 0x2000, CRC(01a7ff6f) SHA1(bfb4ad07d99807eadbb0cb85c5a6cf60a5875f2d) )
	ROM_LOAD( "dog_04.rom", 0x6000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END


ROM_START( nfb96seb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogdptb.prg",0x00000, 0x1000, CRC(0690f915) SHA1(ed2477ba260a421013603017cfd1e1ba5ecd7f4e) ) // alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogmx5",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dogmx6",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dogmx7",   0x00000, 0x08000, CRC(97033c70) SHA1(4d5746f43f8f4d374ba2f31d21defd21921d39bf) ) // wagner video junk on this layer

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogmx1", 0x6000, 0x2000, CRC(b72d2c2c) SHA1(faf60ca0f522868e6dbf7c3ace5c84d8fd001df3) )
	ROM_LOAD( "dogmx2", 0x4000, 0x2000, CRC(a85f5516) SHA1(1564e6c490883c96bffc561d9115eb53450945ce) )
	ROM_LOAD( "dogmx3", 0x2000, 0x2000, CRC(f1a8aea8) SHA1(c20b779a73856d94e862d87ad337c9501da86691) )
	ROM_LOAD( "dogmx4", 0x0000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END

ROM_START( nfb96sec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u34",0x00000, 0x1000, CRC(65e7a70d) SHA1(77b8b7e16e842d6debf9bb13d83dbf4289ab0bd4) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 ) // Same data as other sets, arranged differently
	ROM_LOAD( "u2", 0x00000, 0x08000, CRC(7d245ff8) SHA1(eebc3c0700aaa5e8d0be22d411f097607e179d5e) )
	ROM_CONTINUE(   0x10000, 0x08000 )
	ROM_CONTINUE(   0x08000, 0x08000 )
	ROM_CONTINUE(   0x00000, 0x08000 )

	ROM_REGION( 0x10000, "gfx2", 0 ) // Same data as other sets, arranged differently
	ROM_LOAD( "u35", 0x0000, 0x4000, CRC(ddd6b7cb) SHA1(ff5161d7d25b216fd507a43f9102cde1e1518593) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(    0x2000, 0x2000 )
	ROM_CONTINUE(    0x0000, 0x2000 )
	ROM_LOAD( "u1",  0x4000, 0x4000, CRC(fdbb5896) SHA1(edfcfcfbf6306ff40c517eeeb05f91b869ba40d2) ) // 0xxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(    0x6000, 0x2000 )
	ROM_CONTINUE(    0x4000, 0x2000 )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END

// this contains elephants etc. instead of the usual symbols, maybe
// it's meant to work with the above program roms?
ROM_START( carb2002 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogd.prg",   0x00000, 0x1000, CRC(000102e0) SHA1(a1824576845b67fbc1a9a16d5aafa6cd000ea4fb) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogtai5",   0x10000, 0x08000, CRC(0c8a4afb) SHA1(994295eea7964d60b6a1db911679739a10be9bbe) )
	ROM_LOAD( "dogtai6",   0x08000, 0x08000, CRC(e9f5dc36) SHA1(6a4060c901f10202fe935701f1f1087c8477da56) )
	ROM_LOAD( "dogtai7",   0x00000, 0x08000, CRC(73c4c1aa) SHA1(31a70861dc54b442a1e50adf3f013dbc38fbbbb1) )
	// alt. replacements for roms 5+6?, erases the word 'slot' on the title screen?
	ROM_LOAD( "dogdif5",   0x10000, 0x08000, CRC(a1986e44) SHA1(3178de9c6063c9f33878b6070db95b2eeb12ffea) )
	ROM_LOAD( "dogdif6",   0x08000, 0x08000, CRC(a5d389fc) SHA1(3db570c938a387708974f24a110cf25b9b52ac22) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogtai1",    0x6000, 0x2000, CRC(68ed1c26) SHA1(495a8b57c368b0b7c2a618d4f1e980d5187f411a) )
	ROM_LOAD( "dogtai2",    0x4000, 0x2000, CRC(b5e25d9b) SHA1(9374f7662f92c10ca6d1af570eaa4d161173283f) )
	ROM_LOAD( "dogtai3",    0x2000, 0x2000, CRC(df13aeb2) SHA1(942f742a722bab44dd3de270001b60d888c44111) )
	ROM_LOAD( "dogtai4",    0x0000, 0x2000, CRC(170f07ce) SHA1(4b48841f9c5bdf7bfbc05113148666a5bcdd3d35) )

	// taken from new fruit bonus '96, definitely wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, BAD_DUMP CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, BAD_DUMP CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, BAD_DUMP CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END


// same program as dogh set.. different gfx
ROM_START( carb2003 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogd.prg",   0x00000, 0x1000, CRC(000102e0) SHA1(a1824576845b67fbc1a9a16d5aafa6cd000ea4fb) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "timedog5.rom",   0x10000, 0x08000, CRC(cf05b66d) SHA1(a711a86f2a82dd685a379fda0cf7240b2ca2696e) )
	ROM_LOAD( "timedog6.rom",   0x08000, 0x08000, CRC(2d81bdbe) SHA1(56eaa9347014340b902d8f0bc38b719acf56c314) )
	ROM_LOAD( "timedog7.rom",   0x00000, 0x08000, CRC(f8e410e5) SHA1(1edc863902cfb1605aca08f6970f9bd24147ca0b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	// these seem to contain mixed planes of different gfx sets.. not correct
	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "timedog1.rom",   0x6000, 0x2000, CRC(d29e0217) SHA1(df978143ed313b33f848e7337097fe29c1fa4506) )
	ROM_LOAD( "timedog2.rom",   0x4000, 0x2000, CRC(fafb6a51) SHA1(57e79e5efd525cdf5a4475eedfee2c8fc1417b76) )
	ROM_LOAD( "timedog3.rom",   0x2000, 0x2000, CRC(6f305ac7) SHA1(55f0d73b783854584195658ab4e74770bc13ba0b) )
	ROM_LOAD( "timedog4.rom",   0x0000, 0x2000, CRC(807a16fc) SHA1(111e7d171f9278abea666d6ad41b02f2c8bf98d8) )
	ROM_IGNORE(0x2000)

	// taken from new fruit bonus '96, definitely wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, BAD_DUMP CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, BAD_DUMP CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, BAD_DUMP CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END


ROM_START( nfm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fuitprg", 0x00000, 0x01000, CRC(6f6c98cf) SHA1(4641cb2b90d4d21edc65e504584f3ec92fe741c4) )
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0x2000,0x1000) //?
	ROM_CONTINUE(0xb000,0x1000) //?
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0xd000,0x1000) // ?
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "reelgfx", 0 )
	ROM_LOAD( "fruit1", 0x00000, 0x04000, CRC(dd096dae) SHA1(ab34942cfa4fe7d46892819372c42f566c249f8c) )
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_LOAD( "fruit2", 0x04000, 0x04000, CRC(6a37a16f) SHA1(7adb08d3e4de9768a8e41760a044bf52509da211) )
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_LOAD( "fruit3", 0x08000, 0x04000, CRC(f3361ba7) SHA1(1a7b9c4f685656447bd6ce5f361e6e4af63012e3) )
	ROM_CONTINUE(0x8000, 0x4000)
	ROM_LOAD( "fruit4", 0x0c000, 0x04000, CRC(99ac5ddf) SHA1(65b6abb98f3156f4c0c55478d09c612eed5ae555) )

	// do these graphics really belong with this set? a lot of the tiles seem wrong for it
	ROM_REGION( 0x18000, "tilegfx", 0 )
	ROM_LOAD( "fruit5", 0x00000, 0x08000, CRC(a7a8f08d) SHA1(76c93194133ba85c0dde1f364260e16d5b647134) )
	ROM_LOAD( "fruit6", 0x08000, 0x08000, CRC(39d5b89a) SHA1(4cf52fa557ffc792d3e13f7dbb5d45fd617bac85) )
	ROM_LOAD( "fruit7", 0x10000, 0x08000, CRC(3ade6709) SHA1(9cdf2814e50c5433c582fc40265c5df2a16e99e7) )

	ROM_REGION( 0x18000, "proms", 0 ) // colours?
	ROM_LOAD( "fruiprg2", 0x00000, 0x08000, CRC(13925ff5) SHA1(236415a244ef6092834f8080cf0d2e04bbfa2650) )
ROM_END


/* Super Cherry Master.
   Lacks of Dyna copyright. Maybe bootleg.

   Not reels stop options.
   Tried any input on the real hardware without luck.
*/
ROM_START( scmaster )  // all roms unique
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.u6",  0x00000, 0x10000, CRC(4eef290a) SHA1(27cca383de49d5f0072ecdda11591b78727469c6) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c010.u29", 0x00000, 0x20000, CRC(98bd34b6) SHA1(e3ff6536eb421ed2e2f5d4354099078ba4ae5671) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "27c020.u41", 0x00000, 0x40000, CRC(ece34be2) SHA1(fdfaaffb12a7f6de6bf21b46ad50e845abc00734) )
ROM_END


ROM_START( unkch1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin",  0x00000, 0x10000, CRC(30309996) SHA1(290f35f587fdf78dcb4f09403c510deec533c9c2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.bin", 0x00000, 0x20000, CRC(6db245a1) SHA1(e9f85ba29b0af483eae6f999f49f1e431d9d2e27) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.bin", 0x00000, 0x40000, CRC(b2bca15d) SHA1(57747c9c05e5ab54e40cbded2e420dfbfc929ce5) )
ROM_END


ROM_START( unkch2 ) // only gfx2 differs from unkch1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin",  0x00000, 0x10000, CRC(30309996) SHA1(290f35f587fdf78dcb4f09403c510deec533c9c2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.bin", 0x00000, 0x20000, CRC(6db245a1) SHA1(e9f85ba29b0af483eae6f999f49f1e431d9d2e27) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.1", 0x00000, 0x40000, CRC(725b48c7) SHA1(2f21c33fb7d23ad9411e926130a65b75029b9112) )
ROM_END


ROM_START( unkch3 )  // gfx2 is the same as unkch1, others differ
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.3",  0x00000, 0x10000, CRC(902f9e42) SHA1(ac5843089748d457f70ea52d15285a0ccda705ad) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.3", 0x00000, 0x20000, CRC(546929e6) SHA1(f97fe5687f8776f0abe68962a0246c9bbeb6acd1) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.bin", 0x00000, 0x40000, CRC(b2bca15d) SHA1(57747c9c05e5ab54e40cbded2e420dfbfc929ce5) )
ROM_END


ROM_START( unkch4 )  // all roms unique
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.4",  0x00000, 0x10000, CRC(eb191efa) SHA1(3004f26f9af7633df572f609647716cc4ac75990) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.4", 0x00000, 0x20000, CRC(eaec0034) SHA1(6b2d3922873979eafcd4c71c52017263482b82ab) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.4", 0x00000, 0x40000, CRC(ef586512) SHA1(a720e40903dd04b2c498efad40d583618596e048) )
ROM_END


ROM_START( cherry96 )  // all roms unique
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "new_96-16-3.u6",  0x00000, 0x10000, CRC(84d5f2fc) SHA1(e3ed0670350920c661c5a40581966671b8a8c7df) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "new_96-16-2.u29", 0x00000, 0x20000, CRC(dd8f7450) SHA1(c897e5151809d4e0a0b1e46609f07bb0156b489a) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "new_96-16-1.u41", 0x00000, 0x20000, CRC(9ab19bdc) SHA1(2f34789729b5d12f6fa098a29253d5a80aef5b39) )
ROM_END


/*
  Cherry Master '97

   _____________________________________________________________________________________________
  |        F           E                     D                      C            B         A    |
  |   _________    _________      ______________________                      ________          |
  |11| PAL16L8 |  | PAL16L8 |    |         ZILOG        |       _________    |74HC00AN|         |
  |  |_________|  |_________|    |      Z0840006PSC     |      |LH5168N- |   |________|       __|
  |                              |  Z80 CPU   9349  2E  |      |10L      |                   |
  |   _____________              |______________________|      |_________|                   |
  |  |C97.F10      |                                                                         |__
  |10|   M27C512   |                                                                          --|
  |  |_____________|                                                                          --|
  |                               ______________________       ___________    __________      --|
  |                              |C97.D9                |     | N82S135N  |  |74HC374AN |     --|
  |9                             |       M27C4002       |     |___________|  |__________|     --|
  |                              |______________________|                                     --|
  |                                                                                           --|
  |   ______                                                   ___________    __________      --|
  |8 |74HC14|                                                 | N82S135N  |  |74HC374AN |     --|
  |  |______|                        _____________            |___________|  |__________|     --|
  |                                 |             |                                           --|
  |                                 |             |                                           --|
  |   __               _______      |    DYNA     |                                           --|
  |7 |PS|             |LH5168N|     |   DC3000    |            ___________                    --|
  |  |__|             |-10L   |     |   9650 A    |           | 74LS245N  |                   --|
  |                   |_______|     |             |           |___________|                   --|
  |                                 |             |                                           --|
  |                                 |             |            ___________                    --|
  |6                                |_____________|           | 74LS245N  |                   --|
  |                                                  ____     |___________|                   --|
  |                                                 |XTAL|     ___________                    --|
  |5                                                |    |    | 74LS245N  |                   --|
  |                  ________                       |____|    |___________|                   --|
  |                 |SW 1    |   ________________________                                     --|
  |                 |        |  |          DYNA          |     ________       ________        --|
  |4                1________8  |        22A078803       |    |74LS138N|     |MCT1413P|       --|
  |                  ________   |        9606EY001       |    |________|     |________|       --|
  |                 |SW 2    |  |________________________|                                    --|
  |                 |        |                                 ___________    ________        --|
  |3                1________8   ________________________     | 74LS273N  |  |MCT1413P|       --|
  |                  ________   |ORIGINAL                |    |___________|  |________|       --|
  |                 |SW 3    |  |SER. NO. 076600         |                                    --|
  |2                |        |  |                        |     ___________    ________        --|
  |                 1________8  |________________________|    | 74LS273N  |  |MCT1413P|       --|
  |                  ________                                 |___________|  |________|       --|
  |                 |SW 4    |                                                                __|
  |                 |        |                                                               |
  |                 1________8                                                               |
  |                  ________                                                                |__
  |                 |SW 5    |                                                                  |
  |                 |        |                                                                  |
  |                 1________8                                                                  |
  |1                                                                                            |
  |                                                       ____                     ____         |
  |                                                      |    | | | | | | | | | | |    |        |
  |______________________________________________________|    |___________________|    |________|

  PS = PST519A.
  XTAL = unknown.

         SW 1:                 SW 2:                 SW 3:                 SW 4:                 SW 5:
   _________________     _________________     _________________     _________________     _________________
  | ON              |   | ON              |   | ON              |   | ON              |   | ON              |
  | --------------- |   | --------------- |   | --------------- |   | --------------- |   | --------------- |
  ||#|#| | |#| | | ||   ||#| |#| | |#|#|#||   || | | | | |#| | ||   || | |#|#| | | | ||   || | |#|#|#|#| | ||
  ||---------------||   ||---------------||   ||---------------||   ||---------------||   ||---------------||
  || | |#|#| |#|#|#||   || |#| |#|#| | | ||   ||#|#|#|#|#| |#|#||   ||#|#| | |#|#|#|#||   ||#|#| | | | |#|#||
  | --------------- |   | --------------- |   | --------------- |   | --------------- |   | --------------- |
  | 1 2 3 4 5 6 7 8 |   | 1 2 3 4 5 6 7 8 |   | 1 2 3 4 5 6 7 8 |   | 1 2 3 4 5 6 7 8 |   | 1 2 3 4 5 6 7 8 |
  |_________________|   |_________________|   |_________________|   |_________________|   |_________________|

*/
ROM_START( cmast97 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "c97.f10", 0x00000, 0x10000, CRC(1416439f) SHA1(a1e44e5dc67dfb74c5651a5f9da822a9e6c7df6f) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "c97.d9", 0x000000, 0x80000, CRC(c2c14738) SHA1(dd378cb77a7214ffe5fd9ba1dcbc54f6802b0e41) )

	ROM_REGION( 0x200, "proms", 0 ) // bad decoded
	ROM_LOAD( "82s135.c8",  0x000, 0x100, CRC(4b715969) SHA1(9429dc8698f4ff9195e5e975e62546b7b7e2f856) )
	ROM_LOAD( "82s135.c9",  0x100, 0x100, CRC(85883486) SHA1(adcee60f6fc1e8a75c529951df9e5e1ee277e131) )
ROM_END


/*****************************************************************************************

  MEGA LINES
  Fun World.

  CPU:   1x Z80

  Sound: 3x SN76489.
         1x AY-3-8910.

  ROMs:  1x 27C512 for program.
         2x 27C512 for fg graphics.
         2x 27C256 for reels graphics.

  PROMs: 3x TBP24S10N
         1x N82S123AN

  Clock: 1x Xtal 12.000 MHz.


  Other: 1x DIP-40 custom IC silkscreened '06B30P' (I/O?).
         1x DIP-28 custom IC silkscreened '06B49P' (clock divider).
         2x DIP-28 custom IC silkscreened '06B53P' (video).
         1x 3.6V lithium battery.
         4x 8 DIP switches banks.
         1x Reset switch.

  Connectors: 1x (2x36) edge connector.
              1x (2x10) edge connector.


  ROMS have Falcon original stickers.
  PCB is original from WING Co.,Ltd, with Wing license seal,
  and Eagle & Fun World stickers (serial numbers).


  For custom IC's 06B49P, and 06B53P, see the reverse-engineering notes in lucky74.cpp.

  ------------------------

  IO mapped devices:

  PSGs:   A0 - C0 - E0
  AY8910: 60 - 80

*****************************************************************************************

   PCB Layout

  .--------------------------------------------------------------------------------------------------------------------------.
  |                                                                                                    .---------.   .---.   |
  | .-----------------.  .-----------.   .---------.   .---------.     .-------------------------.     |  3.6 V  |   | o XXXXXX
  | |M27C512          |  |HD74LS245P | R |HD74LS157|   |HD74LS157|     |    Z8400BB1             |     |         |   '---'   |
  | |                 |  '-----------'   '---------'   '---------'     |    Z80BCPU              |     | BATTERY |  RESET SW |
  | |               18|                                                |    29017                |     '---------'       .---'
  | '-----------------'  .-----------.   .---------.   .---------.     '-------------------------'     .--------.        |
  |                      |HD74HC245P | P |HD74LS157|   |HD74LS157|                                     |  NEC   |        |
  | .-----------------.  '-----------'   '---------'   '---------'     .-------------------------.     |uPC1241H|        |
  | |                 |                                                |    06B30P               |     '++++++++'        '---.
  | |  HM6264ALP-12   |  .-----------.   .---------.   .---------.     |    062002               |      ||||||||          ---|
  | |                 |  |HD74HC374P | N |HD74LS157|   |HD74LS157|     |                         |                        ---|
  | '-----------------'  '-----------'   '---------'   '---------'     '-------------------------'                        ---|
  |                                                                                                                       ---|
  | .-----------------.  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  | |                 |  |HD74HC374P | M |TBP24S10N|   |HD74LS138|     |HD74HC245P |   |   SW1   |                        ---|
  | |  06B53P         |  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | |  9G1            |                                                               ----------- DAN803                  ---|
  | '-----------------'  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  |                      |HD74LS273P | L |HD74LS283|   |HD74LS138|     |HD74HC240P |   |   SW2   |                        ---|
  | .-----------------.  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | |M27256ZB         |                                                               ----------- DAN803                  ---|
  | |                 |  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  | |               14|  |HD74HC273P | K |HD74LS283|   |HD74LS139|     |HD74LS273P |   |   SW3   |                        ---|
  | '-----------------'  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  |                                                                                   ----------- DAN803                  ---|
  | .-----------------.  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  | |M27256ZB         |  |HD74HC273P | J | HD7425P |   |N82S123AN|     |HD74LS273P |   |   SW4   |                        ---|
  | |                 |  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | |               13|                                                               ----------- DAN803                  ---|
  | '-----------------'                                                                                                   ---|
  |                      .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  | .-----------------.  |HD74HC245P | H |HD74HC02P|   |TBP24S10N|     |HD74HC273P |   |SN76489AN|                        ---|
  | |                 |  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | |  HM6264ALP-12   |                                                                                                   ---|
  | |                 |                                                                                                   ---|
  | '-----------------'  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  |                      |HD74LS273P | F |HD74LS08P|   |TBP24S10N|     |HD74HC273P |   |SN76489AN|                        ---|
  | .-----------------.  '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | |                 |                                                                                        2x36 EDGE  ---|
  | |  06B53P         |                                                                                        CONNECTOR  ---|
  | |  9G1            |  .-----------.   .---------.   .---------.     .-----------.    ---------.                        ---|
  | '-----------------'  |HD74HC245P | E |HD74LS32P|   |HD74LS86P|     |HD74HC273P |   |SN76489AN|                        ---|
  |                      '-----------'   '---------'   '---------'     '-----------'   '---------'                        ---|
  | .-----------------.                                                                                                   ---|
  | |HN27512P         |  .-----------.   .---------.   .---------.     .-----------.   .---------.                        ---|
  | |                 |  |HD74HC273P | D |HD74LS86P|   |HD74LS86P|     |HD74HC240P |   | NE556N  |                        ---|
  | |               12|  '-----------'   '---------'   '---------'     '-----------'   '---------'                       .---'
  | '-----------------'                                                                                                  |
  |                                                                                                                      |
  | .-----------------.  .-----------.   .---------.   .---------.                                                       |
  | |HN27512P         |  |HD74HC273P | C |HD74LS86P|   |HD74LS74A|                                                       '---.
  | |                 |  '-----------'   '---------'   '---------'                                                           |
  | |               11|                                                                                                      |
  | '-----------------'  .-----------.   .---------.   .---------.     .----.                                                |
  |                      |HD74HC244P | B |HD74LS08P|   |HD74LS74A|     |8212|                                                |
  | .-----------------.  '-----------'   '---------'   '---------'     |CPA |                                                |
  | |                 |                                .---------.     '----'                                                |
  | |  06B49P         |  .-----------.   .---------.   |   XTAL  |                          6               7                |
  | |  8L1            |  |HD74HC240P | A |HD74LS164|   | VX-8111 |                                                           |
  | '-----------------'  '-----------'   '---------'   |12.000Mhz|                             2x10 EDGE CONN                |
  |                                                    '---------'                     .-----.                 .-----.       |
  |          1                 2              3             4               5          |     | | | | | | | | | |     |       |
  '------------------------------------------------------------------------------------'     '-----------------'     '-------'


  IC POSITION (PIN)

                  D1(12)<---+ +--->D1(28)                                  K1(12)<--+ +-->K1(28)
                D1(11)<---+ | | +--->D1(13)                              K1(11)<--+ | | +-->K1(13)
             GND<-------+ | | | | +--->D1(15)                          L2(15)<--+ | | | | +-->K1(15)
           F4(4)<-----+ | | | | | | +--->D1(16)                      J4(13)<--+ | | | | | | +-->K1(16)
         F4(7)<-----+ | | | | | | | | +--->D1(17)                  J4(12)<--+ | | | | | | | | +--->K1(17)
       F4(6)<-----+ | | | | | | | | | | +--->D1(18)              J4(11)<--+ | | | | | | | | | | +--->K1(18)
     F4(5)<-----+ | | | | | | | | | | | | +--->D1(19)          J4(10)<--+ | | | | | | | | | | | | +--->K1(19)
                | | | | | | | | | | | | | |                             | | | | | | | | | | | | | |
                | | | | | | | | | | | | | |                             | | | | | | | | | | | | | |
              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
              |28                         15|                         |28                         15|
              |                             |                         |                             |
              |    [E1]                     |                         |    [M1]                     |
              |>       06B53P               |                         |>       06B53P               |
              |                             |                         |                             |
              |                             |                         |                             |
              | 1                         14|                         | 1                         14|
              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                | | | | | | | | | | | | | |                             | | | | | | | | | | | | | |
                | | | | | | | | | | | | | |                             | | | | | | | | | | | | | |
        C3(8)<--+ | | | | | | | | | | | | +-->B1(19)            C3(8)<--+ | | | | | | | | | | | | +-->J1(19)
          B3(6)<--+ | | | | | | | | | | +-->B1(18)                B3(6)<--+ | | | | | | | | | | +-->J1(18)
           A2(12)<--+ | | | | | | | | +-->B1(17)                   A2(12)<--+ | | | | | | | | +-->J1(17)
              A3(6)<--+ | | | | | | +-->B1(16)                        A3(6)<--+ | | | | | | +-->J1(16)
                B3(2)<--+ | | | | +-->B1(15)                            B3(2)<--+ | | | | +-->J1(15)
                 B1(11)<--+ | | +-->B1(13)                               J1(11)<--+ | | +-->J1(13)
                      GND<--+ +-->B1(12)                                    GND<----+ +-->J1(12)


  ==========================
  Custom 06B53P 9G1 - DIP 28
  ==========================
  Pinout
  ======

  Position [E1]    Position [M1]
  -------------    -------------
  01 -> C3(08)     01 -> C3(08)
  02 -> B3(06)     02 -> B3(06)
  03 -> A2(12)     03 -> A2(12)
  04 -> A3(06)     04 -> A3(06)
  05 -> B3(02)     05 -> B3(02)
  06 -> B1(11)     06 -> J1(11)
  07 -> GND        07 -> GND
  08 -> B1(12)     08 -> J1(12)
  09 -> B1(13)     09 -> J1(13)
  10 -> B1(15)     10 -> J1(15)
  11 -> B1(16)     11 -> J1(16)
  12 -> B1(17)     12 -> J1(17)
  13 -> B1(18)     13 -> J1(18)
  14 -> B1(19)     14 -> J1(19)
  -------------    -------------
  15 -> D1(19)     15 -> K1(19)
  16 -> D1(18)     16 -> K1(18)
  17 -> D1(17)     17 -> K1(17)
  18 -> D1(16)     18 -> K1(16)
  19 -> D1(15)     19 -> K1(15)
  20 -> D1(13)     20 -> K1(13)
  21 -> D1(28)     21 -> K1(28)
  22 -> D1(12)     22 -> K1(12)
  23 -> D1(11)     23 -> K1(11)
  24 -> GND        24 -> L2(15)
  25 -> F4(04)     25 -> J4(13)
  26 -> F4(07)     26 -> J4(12)
  27 -> F4(06)     27 -> J4(11)
  28 -> F4(05)     28 -> J4(10)


  ======================
  Custom 06B30P - DIP 40
  ======================
  Pinout
  ======

  Position [N5]
  ----------------------------
  01 -> Not Connected
  02 -> Not Connected
  03 -> Not Connected
  04 -> N2(05)
  05 -> N2(16)
  06 -> N2(02)
  07 -> M2(19)
  08 -> M2(16)
  09 -> M2(15)
  10 -> M2(12)
  11 -> N2(19) -> 10k -> +5V
  12 -> GND
  13 -> GND
  14 -> M2(06)
  15 -> M2(09)
  16 -> A2(05)
  17 -> 100p -> GND
  18 -> Not Connected
  19 -> H3(03)
  20 -> Not Connected
  ----------------------------
  21 -> GND
  22 -> H3(02)
  23 -> Not Connected
  24 -> +5V
  25 -> N2(09)
  26 -> N2(12)
  27 -> N2(15)
  28 -> N2(06)
  29 -> Not Connected
  30 -> Not Connected
  31 -> P2(18)
  32 -> P2(17)
  33 -> P2(15)
  34 -> P2(18)
  35 -> M2(02)
  36 -> P2(14)
  37 -> P2(13)
  38 -> P2(12)
  39 -> P2(11)
  40 -> Z8400(04)


  ==========================
  Custom 06B49P 8L1 - DIP 28
  ==========================
  Pinout
  ======

  Position [A1]
  --------------------
  01 -> A2(07)
  02 -> A2(15)
  03 -> Not Connected
  04 -> A2(17)
  05 -> A3(01)
  06 -> E4(01)
  07 -> GND
  08 -> E4(12)
  09 -> E4(09)
  10 -> E4(04)
  11 -> D4(09)
  12 -> D4(04)
  13 -> +5V
  14 -> GND
  --------------------
  15 -> GND
  16 -> Not Connected
  17 -> D4(12)
  18 -> D4(01)
  19 -> D3(09)
  20 -> D3(12)
  21 -> +5V
  22 -> D3(01)
  23 -> D3(04)
  24 -> c3(05)
  25 -> C3(01)
  26 -> B3(13)
  27 -> A2(2)
  28 -> B2(04)


*****************************************************************************************

  ========
   BPROMS:
  ========

            GND-------------+ +----------N3(10)
            GND-----------+ | | +--------N3(06)
            K5(06)------+ | | | | +------(N/C)
            +5V-------+ | | | | | | +----L2(13)
                      | | | | | | | |
                    +-----------------+
                    |#################|
                    |>## 24S10 @M3 ###|
                    |#################|
                    +-+-+-+-+-+-+-+-+-+
                      | | | | | | | |
            K5(15)----+ | | | | | | +----GND
            K3(12)------+ | | | | +------D3(03)
            K3(14)--------+ | | +--------D3(11)
            D3(06)----------+ +----------D3(08)



                   J3(10)---+ +----J3(11)
                 K5(05)---+ | | +----J3(12)
              H3(05)----+ | | | | +-----J3(13)
            +5V-------+ | | | | | |
                      | | | | | | | +-------------------+
                      | | | | | | | |                   |
                    +-----------------+                 |
                    |#################|                 |
                    |>## 82S123 @J4 ##|                 |
                    |#################|                 |
                    +-+-+-+-+-+-+-+-+-+                 |
                      | | | | | | | |                   |
                      | | | | | | | +--GND              |
                      | | | | | | +-------------------+ |
                      | | | | | +-------------------+ | |
                      | | | | +-------------------+ | | |
                      | | | +-------+             | | | |
                      | | +-------+ |             | | | |
                      | +-------+ | |             | | | |
                      +-------+ | | |             | | | |
      +---------------------+ | | | |             | | | |
    +-|-------------------+ | | | | |             | | | |
  +-|-|-----------------+ | | | | | |             | | | |
  | | |          +5V--+ | | | | | | |             | | | |
  | | |               | | | | | | | |             | | | |
  | | |             +-----------------+           | | | |
  | | |             |#################|           | | | |
  | | |             |>## 24S10 @H4 ###|           | | | |
  | | |             |#################|           | | | |
  | | |             +-+-+-+-+-+-+-+-+-+           | | | |
  | | |               | | | | | | | |             | | | |
  | | |   +-----------|-|-+ | | | | +--GND        | | | |
  | | |   | +---------|-+   | | | +--------+      | | | |
  | | |   | | +-------+     | | +----------|-+    | | | |
  | | | +-|-|-|-------------+ +------------|-|-+  | | | |
  | | | | | | |                            | | |  | | | |
  | | +-|-|-|-|-------------+ +------------|-|-|--+ | | |
  | +---|-|-|-|-----------+ | | +----------|-|-|----+ | |
  +-----|-|-|-|---------+ | | | | +--------|-|-|------+ |
        | | | |  +5V--+ | | | | | | +------|-|-|--------+
        | | | |       | | | | | | | |      | | |
        | | | |     +-----------------+    | | |
        | | | |     |#################|    | | |
        | | | |     |>## 24S10 @F4 ###|    | | |
        | | | |     |#################|    | | |
        | | | |     +-+-+-+-+-+-+-+-+-+    | | |
        | | | |       | | | | | | | |      | | |
        | | | +-------+ | | | | | | +--GND | | |
        | | +-----------+ | | | | +--------+ | |
        | +---------------+ | | +------------+ |
        +-------------------+ +----------------+


  ==============
   DIP SWITCHES:
  ==============
                       +----------+--------------------------------------+
                     +-|----------|-+----------------------------------+ |
                   +-|-|----------|-|-+------------------------------+ | |
                 +-|-|-|----------|-|-|-+--------------------------+ | | |
               +-|-|-|-|----------|-|-|-|-+----------------------+ | | | |
             +-|-|-|-|-|----------|-|-|-|-|-+------------------+ | | | | |
           +-|-|-|-|-|-|----------|-|-|-|-|-|-+--------------+ | | | | | |
         +-|-|-|-|-|-|-|----------|-|-|-|-|-|-|-+----------+ | | | | | | |
         | | | | | | | |          | | | | | | | |          | | | | | | | |
         | | | | | | | |      +---+-+-+-+-+-+-+-+---+      | | | | | | | |
   20| | | | | | | | | |      | 1+---------------+8 |      | | | | | | | |
   +-+-+-+-+-+-+-+-+-+-+-+    |  | | | | | | | | |  |      | | | | | | | |
   |#####################|    |  -----------------  |SW1   | | | | | | | |
   |>#  HD74HC245P @M5  #|    |  |#|#|#|#|#|#|#|#|  |      | | | | | | | |
   |#####################|    |  +---------------+  |      | | | | | | | |
   +-+-+-+-+-+-+-+-+-+-+-+    +---+-+-+-+-+-+-+-+---+      | | | | | | | |
    1| | | | | | | | | |          | | | | | | | |          | | | | | | | |
             |                  +-+-+-+-+-+-+-+-+-+        | | | | | | | |
             +------------------+#### DAN803 #####|        | | | | | | | |
                                +-----------------+        | | | | | | | |
                                                           | | | | | | | |
                                  +------------------------|-|-|-|-|-|-|-+
                                  | +----------------------|-|-|-|-|-|-+ |
                                  | | +--------------------|-|-|-|-|-+ | |
                                  | | | +------------------|-|-|-|-+ | | |
                                  | | | | +----------------|-|-|-+ | | | |
                                  | | | | | +--------------|-|-+ | | | | |
                                  | | | | | | +------------|-+ | | | | | |
                                  | | | | | | | +----------+ | | | | | | |
                                  | | | | | | | |          | | | | | | | |
                              +---+-+-+-+-+-+-+-+---+      | | | | | | | |
                              | 1+---------------+8 |      | | | | | | | |
                              |  | | | | | | | | |  |      | | | | | | | |
                              |  -----------------  |SW2   | | | | | | | |
                              |  |#|#|#|#|#|#|#|#|  |      | | | | | | | |
                              |  +---------------+  |      | | | | | | | |
                              +---+-+-+-+-+-+-+-+---+      | | | | | | | |
                                  | | | | | | | |          | | | | | | | |
                                +-+-+-+-+-+-+-+-+-+        | | | | | | | |
                     +----------+#### DAN803 #####|        | | | | | | | |
                     |          +-----------------+        | | | | | | | |
   HD74LS139         |                                     | | | | | | | |
      @K4            |            +------------------------|-|-|-|-|-|-|-+
                     |            | +----------------------|-|-|-|-|-|-+ |
   1 +-V-+ 16        |            | | +--------------------|-|-|-|-|-+ | |
    -|###|-          |            | | | +------------------|-|-|-|-+ | | |
    -|###|-          |            | | | | +----------------|-|-|-+ | | | |
    -|###|-          |            | | | | | +--------------|-|-+ | | | | |
    -|###|-          |            | | | | | | +------------|-+ | | | | | |
    -|###|-          |            | | | | | | | +----------+ | | | | | | |
    -|###|-----------+            | | | | | | | |          | | | | | | | |
    -|###|---------+          +---+-+-+-+-+-+-+-+---+      | | | | | | | |
    -|###|--+      |          | 1+---------------+8 |      | | | | | | | |
     +---+  |      |          |  | | | | | | | | |  |      | | | | | | | |
            |      |          |  -----------------  |SW3   | | | | | | | |
            |      |          |  |#|#|#|#|#|#|#|#|  |      | | | | | | | |
            |      |          |  +---------------+  |      | | | | | | | |
            |      |          +---+-+-+-+-+-+-+-+---+      | | | | | | | |
            |      |              | | | | | | | |          | | | | | | | |
            |      |            +-+-+-+-+-+-+-+-+-+        | | | | | | | |
            |      +------------+#### DAN803 #####|        | | | | | | | |
            |                   +-----------------+        | | | | | | | |
            |                                              | | | | | | | |
            |                     +------------------------|-|-|-|-|-|-|-+
            |                     | +----------------------|-|-|-|-|-|-+
            |                     | | +--------------------|-|-|-|-|-+
            |                     | | | +------------------|-|-|-|-+
            |                     | | | | +----------------|-|-|-+
            |                     | | | | | +--------------|-|-+
            |                     | | | | | | +------------|-+
            |                     | | | | | | | +----------+
            |                     | | | | | | | |
            |                 +---+-+-+-+-+-+-+-+---+
            |                 | 1+---------------+8 |
            |                 |  | | | | | | | | |  |
            |                 |  -----------------  |SW4
            |                 |  |#|#|#|#|#|#|#|#|  |
            |                 |  +---------------+  |
            |                 +---+-+-+-+-+-+-+-+---+
            |                     | | | | | | | |
            |                   +-+-+-+-+-+-+-+-+-+
            +-------------------+#### DAN803 #####|
                                +-----------------+


*****************************************************************************************/
ROM_START( megaline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "18.r1",  0x00000, 0x10000, CRC(37234cca) SHA1(f991bc55fbfc69594573608ca03a9001ccf2f73b) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "11.b1",  0x00000, 0x10000, CRC(6e7810d8) SHA1(16f1331851041b971a62f653f69b8853a2c4f868) )
	ROM_LOAD( "12.d1",  0x10000, 0x10000, CRC(054c6ee7) SHA1(6e91223c8f6a2dc93a39a1e6453ccd9c731b8b45) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "13.j1",  0x0000, 0x8000, CRC(5676ccb3) SHA1(36794c365c0b7490a9046422c0b334a3cdc15b8e) )
	ROM_LOAD( "14.k1",  0x8000, 0x8000, CRC(81acfc59) SHA1(b6f94ade557a2d3ba5e358d33e83016a210890e7) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "tbp24s10.f4", 0x0000, 0x0100, CRC(d864b6f1) SHA1(6edc1941fe49cf53f073bf4acc466cd28b788146) )
	ROM_LOAD( "tbp24s10.h4", 0x0100, 0x0100, CRC(4acd5887) SHA1(dca1187a74d9f4abc53b77a1590ec726f682dd91) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "n82s123an.j4", 0x0000, 0x0020, CRC(5447e258) SHA1(04057cddb86896500954b9aa6d6508aa081b9645) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10.m3", 0x0000, 0x0100, CRC(7edb311b) SHA1(8e7f933313dc7a1f2a5e8803c26953ced3f798d0) )
ROM_END

/*
  Bonus Chance (W-8).
  Wing Co. Ltd.

  Board similar to Mega Lines, but with slightly
  different layout and some extra additions...

  CPU:   1x Z80 @ 6MHz. (measured)
  MCU:   1x M80C51F-711 (8-Bit Microcontroller) @ 12MHz. (measured).

  Sound: 4x SN76489 @ 3MHz. (measured).

  ROMs:  1x 27C512 for program.
         2x 27C512 for fg graphics.
         2x 27C512 for reels graphics.

  PROMs: 5x TBP24S10N

  Clock: 1x Xtal 12.000 MHz.


  Other: 1x DIP-40 custom IC silkscreened '06B30P' (I/O?).
         1x DIP-28 custom IC silkscreened '06B49P' (clock divider, measured).
         2x DIP-28 custom IC silkscreened '06B53P' (video).
         1x 3.6V lithium battery.
         5x 8 DIP switches banks.
         1x Reset switch.

  Connectors: 1x (2x36) edge connector.
              1x (2x10) edge connector.


  ROMS have Eagle original stickers.
  PCB is original from WING Co.,Ltd, with Wing license seal,
  and Eagle stickers (serial numbers).


  For custom IC's 06B49P, and 06B53P, see the reverse-engineering notes in lucky74.c.

*/

ROM_START( bonusch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.1u", 0x00000, 0x10000, CRC(5098eba6) SHA1(40a33a25d3589dfe7e228f1874239fbfbc5250e4) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "0.5p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1.1c",  0x00000, 0x10000, CRC(33ce67c0) SHA1(8f3d7e78a4616bebafed2779b7f793d41576c9c8) )
	ROM_LOAD( "2.1e",  0x10000, 0x10000, CRC(fc394767) SHA1(645bf0e60a7061771aa73bb4d10603eaaad17f20) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "3.1m",  0x00000, 0x10000, CRC(2acac012) SHA1(59d879214c4e473fa6fedb4a08dcd9b3c6a881a3) )
	ROM_LOAD( "4.1p",  0x10000, 0x10000, CRC(530bdec2) SHA1(2ce0993386fe6b165363a053b54fc66d8bf385d7) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "tbp24s10.3e", 0x0000, 0x0100, CRC(f8d160c5) SHA1(a3cb9c4337f4f030d62e74ccc882052959b1fa4f) )
	ROM_LOAD( "tbp24s10.3f", 0x0100, 0x0100, CRC(bbc03eb2) SHA1(c0e44df0ec8268344f59965e3b9d62a4dca2ebb2) )
	ROM_LOAD( "tbp24s10.3h", 0x0200, 0x0100, CRC(77b2585d) SHA1(898302f9a0bd8e354794087461f8f1103bb63783) )

	ROM_REGION( 0x300, "proms2", 0 )
	ROM_LOAD( "tbp24s10.4e", 0x0000, 0x0100, CRC(06fa2649) SHA1(b2f17d37826317ccad19d535cd5afeedb143778b) )
	ROM_LOAD( "tbp24s10.4f", 0x0100, 0x0100, CRC(38000593) SHA1(e0113590cb2dc338d61ae2e7e92b1046c5c2d19f) )
	ROM_LOAD( "tbp24s10.4h", 0x0200, 0x0100, CRC(cbf0062d) SHA1(f49dfca34d2eb86b5ff16872fab23d3e3a10be9a) )

ROM_END

/*
Fever Chance
Wing 19?? (1986 in ROM).

Wing license seal but Eagle labeled ROMs

1 x Z80
3 x I8255A
3 x SN76489AN
1 x unknown at 8d (possibly battery backed RAM)
*/

ROM_START( feverch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w9.c11", 0x00000, 0x8000, CRC(4dda18ef) SHA1(9a98a2f6996903b58d53e10b7b68c6ed1c34967a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cf18.7h", 0x00000, 0x08000, CRC(c63924fe) SHA1(7471d05c8688ba1fa6c0c3444de8883595c21776) )
	ROM_LOAD( "cf19.8h", 0x08000, 0x08000, CRC(f0229490) SHA1(665d335cc030a0cbec0c11c685a6f1e2f9706989) )
	ROM_LOAD( "cf20.10h", 0x10000, 0x08000, CRC(1d831a06) SHA1(42d235b8dd894d38579886940a3e13adb843e00d) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "cf1.1h", 0x00000, 0x02000, CRC(5f022073) SHA1(2e154837834cc9db452279b4933900234b568565) ) // 1st and 2nd half identical
	ROM_CONTINUE(0x00000, 0x02000)
	ROM_LOAD( "cf2.2h", 0x02000, 0x02000, CRC(e8f927b9) SHA1(29dec2f21a1bea250a4a2d75fab8d03a1fc70bcd) ) // 1st and 2nd half identical
	ROM_CONTINUE(0x02000, 0x02000)
	ROM_LOAD( "cf3.4h", 0x04000, 0x02000, CRC(79b06e00) SHA1(18f73527714914edb57e22909c95f2c764223900) ) // 1st and 2nd half identical
	ROM_CONTINUE(0x04000, 0x02000)
	ROM_LOAD( "cf4.5h", 0x06000, 0x02000, CRC(7f73744e) SHA1(7c07095f7ec4302a4839a279c755979ec10e0715) ) // 1st and 2nd half identical
	ROM_CONTINUE(0x06000, 0x02000)

	// PROMs not dumped, taken from lucky8
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12",   0x0000, 0x0100, BAD_DUMP CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "prom4", 0x0100, 0x0100, BAD_DUMP CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, BAD_DUMP CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )
ROM_END

/*
  Win Cherry (ver 0.16 - 19990219)

  1x Z0840006PSC-Z80CPU 8-bit Microprocessor.
  1x KC89C72            Programmable Sound Generator.
  1x TDA2003            Audio Amplifier.

  1x 27C256 (ic9) dumped.
  1x 27C512 (ic2) dumped.
  1x M27C1001 (ic6) dumped.
  1x AM27C29PC (not dumped yet).

  1x CY62256LL-70PC Static RAM.
  1x LP6264D-70LL   Static RAM.

  2x Xilinx XC9572-PC84 CPLD's (read protected).

  1x oscillator 12.000 MHz.
  1x 2x28 JAMMA edge connector.
  2x pushbutton (MANAG. - STATIS.).
  1x trimmer (volume).
  5x 8 DIP switches.
  1x battery (missing).

*/

ROM_START( wcherry )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wc20_388.ic2",  0x00000, 0x10000, CRC(b1ea0e6a) SHA1(2dd3f2cfffc1e47b45c29daa9d7df7af956b599c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "wincherryb.ic6",  0x00000, 0x20000, CRC(cace16f5) SHA1(a6caddc6ccd30901e2332a42f339a1da022de410) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "wincherrya.ic9",  0x00000, 0x08000, CRC(919bd692) SHA1(1aeb66f1e4555b731858833445000593e613f74d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27c29pc",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )    // borrowed from other game.
ROM_END

/*
Taiwan Chess Legend
Uniwang, 1995

Preliminary driver by Tomasz Slanina

PCB Layout
----------

|-----------------------------------------------|
|  AY8930   DSW5  TCL.1E                        |
|           DSW4  TCL.3E   TCL.3F  TCL.3H       |
|           DSW3  TCL.4E   TCL.4F  TCL.4H       |
|           DSW2  6116                          |
|           DSW1  6116          6116            |
|                               6116            |
|  8255                               12MHz     |
|                                               |
|                                               |
|                 TCL_PR3.9E                    |
|                                               |
|  8255                     PAL                 |
|                                               |
|                           PAL                 |
|  BATTERY                    LATTICE           |
|                       PAL   PLSI 1016 Z80 PAL |
|           TCL_PR1.15C                     PAL |
|  SW1      TCL_PR2.16C 6116      TCL.16F       |
|-----------------------------------------------|

Notes:
      Z80 Clock: 3.000MHz
          VSync: 60Hz
          HSync: 15.15kHz

 This appears to be based off a Blue Dyna Cherry Master board but with extra protection (the sub-board with CPU)

*/

ROM_START( tcl )
	ROM_REGION( 0x10000*2, "maincpu", 0 )
	ROM_LOAD( "tcl.16f",   0x00000, 0x20000, CRC(8e694a58) SHA1(7a3c20a7c740065b71fe66ec581edce0dd32f145) )

	ROM_REGION( 0x8000*3, "gfx1", 0 )
	ROM_LOAD( "tcl.1e",   0x00000, 0x8000, CRC(37edf9b8) SHA1(9225728116d6edfe8476e565a12e1f1e59766d26) )
	ROM_LOAD( "tcl.3e",   0x08000, 0x8000, CRC(396298cf) SHA1(0ee306179a9d3dd84f7e5799527e6825d2979ddb) )
	ROM_LOAD( "tcl.4e",   0x10000, 0x8000, CRC(f880101c) SHA1(8417410a7dcb304a88e98f9199f44a4df1ee3fb7) )

	ROM_REGION( 0x2000*4, "gfx2", 0 ) /* ??? */
	ROM_LOAD( "tcl.3f",   0x0000, 0x2000, CRC(c290c1eb) SHA1(00eb5ff46affe01f240081211b7f9a40e9f76bd8) )
	ROM_LOAD( "tcl.4f",   0x2000, 0x2000, CRC(225e0148) SHA1(26d8db263b1957fc6b2204765c8aa1f10f44b591) )
	ROM_LOAD( "tcl.3h",   0x4000, 0x2000, CRC(ee63d380) SHA1(c1d9ca4584bb2ef0fa85e2afb0876040b473a924) )
	ROM_LOAD( "tcl.4h",   0x6000, 0x2000, CRC(6afa36a1) SHA1(a87423f01dbf9b1e69feb049d6ae3fd63321ee1a) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF ) // (girl bitmaps, not present)

	ROM_REGION( 0x100*3, "proms", 0 )
	ROM_LOAD( "tcl_pr1.15c",   0x000, 0x100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )
	ROM_LOAD( "tcl_pr2.16c",   0x100, 0x100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "tcl_pr3.9e",    0x200, 0x100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/****************************** Stealth Sets ******************************/

/*
  Super PacMan 2.1 + Cherry Master (Corsica, ver 8.31)
  stealth game...

  The game runs in the blue old cherry master hardware,
  with a daughterboard placed in the Z80 socket.

  You can switch the games pulsing a hidden input.

  Hidden Switch:

  One wire goes to edge connector 35+36 (solder side),
  carrying GND, and putting PPI (u34) pin 14 (PC0) to GND.

  The other goes to edge connector 8 (solder side): PPI pin 18 (PB0)

*/
ROM_START( cmpacman )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Seems to have the pulse/bookkeeping bug fixed.
	ROM_LOAD( "corsica_v8.31_pacman_old_board_new.bin",  0x0000,  0x10000, CRC(4f1ea727) SHA1(e127f3da9e7cc81b93dc3eec66ca56452e78375e) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "c_m_pacman_rom7.u16", 0x00000,  0x8000, CRC(c53273a4) SHA1(d359e65c31ef5253f1e9a3b67db8851a8d1262d1) )
	ROM_LOAD( "c_m_pacman_rom6.u11", 0x08000,  0x8000, CRC(013bff64) SHA1(65f2808480970a756b642ddd1a64c10b89ea3b3e) )
	ROM_LOAD( "c_m_pacman_rom5.u4",  0x10000,  0x8000, CRC(03298f22) SHA1(32c99da82afff6d38333a9998802c497d6f49fab) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmpacmana )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Seems to have a pulse/bookkeeping bug.
	ROM_LOAD( "corsica_v8.31_pacman_old_board.bin",  0x0000,  0x10000, CRC(f69cbe75) SHA1(08446eb005b6c7ed24489fd664df14b20a41e3eb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "c_m_pacman_rom7.u16", 0x00000,  0x8000, CRC(c53273a4) SHA1(d359e65c31ef5253f1e9a3b67db8851a8d1262d1) )
	ROM_LOAD( "c_m_pacman_rom6.u11", 0x08000,  0x8000, CRC(013bff64) SHA1(65f2808480970a756b642ddd1a64c10b89ea3b3e) )
	ROM_LOAD( "c_m_pacman_rom5.u4",  0x10000,  0x8000, CRC(03298f22) SHA1(32c99da82afff6d38333a9998802c497d6f49fab) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*
  Tetris + Cherry Master (Corsica, ver 8.01)
  stealth game...

  The game runs in the blue old cherry master hardware,
  with some mods...

  You can switch the games pulsing their respective hidden buttons.

  Hidden Switch:

  One wire goes to edge connector 35+36 (solder side), putting PPI (u34)
  pin 14 (PC0) to GND (needed to get the switch working).

  One button adds GND to the edge connector 8 (solder side):
  PPI pin 18 (PB0), to switch to Tetris game.

  The other button, adds GND to the edge connector 7 (solder side):
  PPI pin 19 (PB1), to switch to Cherry Master game.

*/
ROM_START( cmtetris )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tetris_cm_v8.01.u81",  0x0000,  0x10000, CRC(3f67dcb2) SHA1(00aa1dcfd14f9b3e130ad31462cd7c9873e9a990) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7__cmtetris.u16", 0x00000,  0x8000, CRC(2f5c94bd) SHA1(d99bcaa788f8abf5c75b29572d53be109b20c4bb) )
	ROM_LOAD( "6__cmtetris.u11", 0x08000,  0x8000, CRC(dac50071) SHA1(7d1c8ec0d81897fe2155578d8c7455dc07104899) )
	ROM_LOAD( "5__cmtetris.u4",  0x10000,  0x8000, CRC(9d67e265) SHA1(62eba137d881789c70121d5c07b5247684b917dd) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*
  Tetris + Cherry Master (Corsica, ver 8.01)
  stealth game...

  The game runs in the blue old cherry master hardware,
  with some mods... The program is shuffled in banks of 0x1000.

  You can switch the games pulsing their respective hidden buttons.

  The Tetris game is different to the one in cmtetris set.

  Hidden Switch:

  One wire goes to edge connector 35+36 (solder side), putting PPI (u34)
  pin 14 (PC0) to GND (needed to get the switch working).

  One button adds GND to the edge connector 8 (solder side):
  PPI pin 18 (PB0), to switch to Tetris game.

  The other button, adds GND to the edge connector 7 (solder side):
  PPI pin 19 (PB1), to switch to Cherry Master game.

*/
ROM_START( cmtetrsa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cm89-tetri-9.u81",  0x10000,  0x10000, CRC(75e0c101) SHA1(6dc4f7c43f0f4e21d621f3c42cb1709d6b730c53) )
/*
  Need checks and fixes

  3800+ --> 5800+
  9800+ --> 3800+

*/
//  ROM_COPY( "maincpu", 0x1c000, 0x0000, 0x1000 )      /* src-dest-size ok */
//  ROM_COPY( "maincpu", 0x16000, 0x1000, 0x1000 )      /* src-dest-size ok */
//  ROM_COPY( "maincpu", 0x14000, 0x2000, 0x1000 )      /* src-dest-size ok */
//  ROM_COPY( "maincpu", 0x1a000, 0x3000, 0x1000 )      /* src-dest-size ok (some calls to high 5xxx appear here, maybe split in 0x800?) */
//  ROM_COPY( "maincpu", 0x15000, 0x4000, 0x1000 )      /* src-dest-size ok */
//  ROM_COPY( "maincpu", 0x11000, 0x6000, 0x1000 )      /* src-dest-size ok */
//  ROM_COPY( "maincpu", 0x13000, 0x8000, 0x1000 )      /* src-dest-size ok */

//  ROM_COPY( "maincpu", 0x17000, 0x5000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x10000, 0x7000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x18000, 0x9000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x19000, 0xa000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x12000, 0xb000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x1b000, 0xc000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x1d000, 0xd000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x1e000, 0xe000, 0x1000 )      /* src-dest-size */
//  ROM_COPY( "maincpu", 0x1f000, 0xf000, 0x1000 )      /* src-dest-size */

	ROM_COPY( "maincpu", 0x1c000, 0x0000, 0x0800 )      /* src-dest-size */ // #01
	ROM_COPY( "maincpu", 0x19800, 0x0800, 0x0800 )      /* src-dest-size */ // #02
	ROM_COPY( "maincpu", 0x16000, 0x1000, 0x0800 )      /* src-dest-size */ // #03
	ROM_COPY( "maincpu", 0x17800, 0x1800, 0x0800 )      /* src-dest-size */ // #04
	ROM_COPY( "maincpu", 0x14000, 0x2000, 0x0800 )      /* src-dest-size */ // #05
	ROM_COPY( "maincpu", 0x1c800, 0x2800, 0x0800 )      /* src-dest-size */ // #06
	ROM_COPY( "maincpu", 0x1a000, 0x3000, 0x0800 )      /* src-dest-size */ // #07
	ROM_COPY( "maincpu", 0x18800, 0x3800, 0x0800 )      /* src-dest-size */ // #08
	ROM_COPY( "maincpu", 0x10000, 0x4000, 0x0800 )      /* src-dest-size */ // #09
	ROM_COPY( "maincpu", 0x15000, 0x4800, 0x0800 )      /* src-dest-size */ // #10
	ROM_COPY( "maincpu", 0x14800, 0x5000, 0x0800 )      /* src-dest-size */ // #11
	ROM_COPY( "maincpu", 0x1a800, 0x5800, 0x0800 )      /* src-dest-size */ // #12
	ROM_COPY( "maincpu", 0x11000, 0x6000, 0x0800 )      /* src-dest-size */ // #13
	ROM_COPY( "maincpu", 0x11800, 0x6800, 0x0800 )      /* src-dest-size */ // #14
	ROM_COPY( "maincpu", 0x1b000, 0x7000, 0x0800 )      /* src-dest-size */ // #15
	ROM_COPY( "maincpu", 0x1f000, 0x7800, 0x0800 )      /* src-dest-size */ // #16
	ROM_COPY( "maincpu", 0x1f800, 0x8000, 0x0800 )      /* src-dest-size */ // #17
	ROM_COPY( "maincpu", 0x13800, 0x8800, 0x0800 )      /* src-dest-size */ // #18
	ROM_COPY( "maincpu", 0x19000, 0x9000, 0x0800 )      /* src-dest-size */ // #19
	ROM_COPY( "maincpu", 0x1b800, 0x9800, 0x0800 )      /* src-dest-size */ // #20
	ROM_COPY( "maincpu", 0x12000, 0xa000, 0x0800 )      /* src-dest-size */ // #21
	ROM_COPY( "maincpu", 0x10800, 0xa800, 0x0800 )      /* src-dest-size */ // #22
	ROM_COPY( "maincpu", 0x18000, 0xb000, 0x0800 )      /* src-dest-size */ // #23
	ROM_COPY( "maincpu", 0x12800, 0xb800, 0x0800 )      /* src-dest-size */ // #24
	ROM_COPY( "maincpu", 0x13000, 0xc000, 0x0800 )      /* src-dest-size */ // #25

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cm89-tetri-7.u16", 0x00000,  0x8000, CRC(2f5c94bd) SHA1(d99bcaa788f8abf5c75b29572d53be109b20c4bb) )
	ROM_LOAD( "cm89-tetri-6.u11", 0x08000,  0x8000, CRC(dac50071) SHA1(7d1c8ec0d81897fe2155578d8c7455dc07104899) )
	ROM_LOAD( "cm89-tetri-5.u4",  0x10000,  0x8000, CRC(9d67e265) SHA1(62eba137d881789c70121d5c07b5247684b917dd) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "cm89-tetri-4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "cm89-tetri-3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "cm89-tetri-2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "cm89-tetri-1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "cm89-tetri-8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*
  Tetris + Cherry Master (+K, Canada Version, encrypted)
  stealth game...

  The game runs in the blue old cherry master hardware,
  with a daughterboard placed in the Z80 socket.

  Daughterboard specs:

  Silkscreened: AMUS89 REV.0

  1x Zilog Z0840006PSC (Z80 CPU)
  1x Lattice A444A12 (PLCC68)
  1x Unknown DIL-24 IC marked 3567 HX881(1) 9620.
  1x AM27C512 for program, labeled '9'.
  1x HM6116LP-3
  1x C358C (dual oper amplifier)

  1x 12.000 MHz. crystal.
  1x 3.579545 MHz. crystal.


  About the unknown IC....

  HX881 is known as U3567, U3567 is a YM2413 clone.
  YM2413 is 18 pins, but the SOP package is 24 pins.
  Because of the dual amp, this might support it being a sound chip.
  Probably tracing pin connections to the HX881 will confirm it.


  Hidden Switch:

  You can enter Cherry Master mode in same way like other Tetris versions.

  One wire goes to edge connector 35 + 36 (solder side), putting PPI (u34)
  pin 14 (PC0) to GND (needed to get the switch working).

  One button adds GND to the edge connector 8 (solder side):
  PPI pin 18 (PB0), to switch to Tetris game.

  The other button, adds GND to the edge connector 7 (solder side):
  PPI pin 19 (PB1), to switch to Cherry Master game.

  Need proper implementation.

*/
ROM_START( cmtetrsb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9__canada_daughterboard.bin",  0x0000,  0x10000, CRC(9810b853) SHA1(cf1216414f93cc78c7c9e5a3998e8b162692e05e) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "rom7.u16", 0x00000,  0x10000, CRC(51498501) SHA1(7ac92129b449f7d4407f847c6200bf278c196c02) )
	ROM_LOAD( "rom6.u11", 0x10000,  0x10000, CRC(8b113a3c) SHA1(aa24edd672c05a06f476286b343f8b1a40d5f0c9) )
	ROM_LOAD( "rom5.u4",  0x20000,  0x10000, CRC(c3de5ce1) SHA1(2945c8e336c6d8638899b798fbe79c5757941fd8) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "rom4.u15",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "rom3.u10",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "rom2.u14",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "rom1.u9",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "rom8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u43", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/********************** Flaming 7, from Cyberdyne Systems, Inc. ***********************

  Flaming 7.
  Cyberdyne Systems, Inc.

  Graphics: CGA
  Reels: 3 spinning reels.
  Lines: 8.
  Bonus Games: Yes.
  Harness Type: Standard 36/10 Pin Cherry Master Harness.

  Looks like a Wing W-4 derivative. There are at least 3 different hardwares:
  Cyberdyne original PCB, Taiwanese custom PCB, and one close to Wing W-4.

  Graphics: 4 different types, selectable through DIP switches.

  1) Regular Fruits.
  2) Numbers.     (Custom Option 1 in W-4 HW)
  3) Custom Set 1 (Custom Option 2 in W-4 HW)
  4) Custom Set 2 (Custom Option 3 in W-4 HW)

  Titles / GFX upgrades:

  +----+------------------------------+---------+----------+
  | N# |      Title:                  | Dumped  | Emulated |
  +----+------------------------------+---------+----------+
  | 01 | Big Bull Frog.               | NO      | NO       |
  | 02 | Buffalo Bills.               | NO      | NO       |
  | 03 | Cash Cabaret.                | NO      | NO       |
  | 04 | Cash Cow.                    | NO      | NO       |
  | 05 | Cherry 50 Bonus.             | YES     | YES      |  Custom main 50, 500 & 2000.
  | 06 | Cherry 500 Bonus.            | YES     | YES      |  Custom main 50, 500 & 2000.
  | 07 | Cherry 1000 Bonus.           | YES     | YES      |  Custom main 50, 500 & 2000.
  | 08 | Christmas.                   | NO      | NO       |
  | 09 | Deuces Wild.                 | NO      | NO       |
  | 10 | Diamond Delight.             | YES     | YES      |  Different set. Custom main 50, 500 & 2000.
  | 11 | Diamond Doubles.             | NO      | NO       |
  | 12 | Diamond Treasure.            | NO      | NO       |
  | 13 | Dream Catcher.               | NO      | NO       |
  | 14 | Dynamite Diamonds 1.         | NO      | NO       |
  | 15 | Dynamite Diamonds 2.         | NO      | NO       |
  | 16 | Egyptian Gold.               | NO      | NO       |
  | 17 | Gold Country.                | NO      | NO       |
  | 18 | Golden Treasure.             | NO      | NO       |
  | 19 | Greenbacks.                  | NO      | NO       |
  | 20 | Harley Davidson.             | NO      | NO       |
  | 21 | Hollywood Nights.            | YES (W4)| YES (W4) |  Flaming 7's W4 version.
  | 22 | Independence Day.            | NO      | NO       |
  | 23 | Jokers Wild.                 | NO      | NO       |
  | 24 | Midnight Sevens.             | NO      | NO       |
  | 25 | Mighty Buffalo.              | NO      | NO       |
  | 26 | Money.                       | NO      | NO       |
  | 27 | Moola.                       | NO      | NO       |
  | 28 | New Year's.                  | NO      | NO       |
  | 29 | Prospector's Gold.           | NO      | NO       |
  | 30 | Red Hot Ice.                 | NO      | NO       |
  | 31 | Red, White & Blue 7's.       | YES (W4)| YES (W4) |  Flaming 7's W4 version.
  | 32 | Rising Star.                 | NO      | NO       |
  | 33 | Rockin' Reels.               | NO      | NO       |
  | 34 | Rolling Thunder.             | NO      | NO       |
  | 35 | Soboba Gold.                 | NO      | NO       |
  | 36 | Star Sevens / American Gold. | NO      | NO       |
  | 37 | Sun God.                     | NO      | NO       |
  | 38 | Super Stars & Stripes.       | NO      | NO       |
  | 39 | Sweet Dreams.                | NO      | NO       |
  | 40 | Sweethearts & Sevens.        | NO      | NO       |
  | 41 | Tropical Treasure.           | NO      | NO       |
  | 42 | Vegas Classic.               | NO      | NO       |
  | 43 | White Buffalo.               | NO      | NO       |
  | 44 | Wild Thing.                  | NO      | NO       |
  | 45 | Wild Watermelons.            | NO      | NO       |
  | 46 | Wild West.                   | NO      | NO       |
  +----+------------------------------+---------+----------+

  Will name the sets using the internal code of both custom
  graphics sets in their respective order...

  -------------------------------------------------------------------

  Original Cyberdyne Systems hardware DIP switches:

  +-----------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIP SWITCHES BANK #1              |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | CREDITS OUT    | NO CREDITS OUT   | OFF | OFF |     |     |     |     |     |     |
  |                | TICKET PRINTER   | ON  | OFF |     |     |     |     |     |     |
  |                | HOPPER PAYOUT    | OFF | ON  |     |     |     |     |     |     |
  |                | REMOTE CLEAR     | ON  | ON  |     |     |     |     |     |     |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | GAME SPEED     | FAST GAME        |     |     | OFF |     |     |     |     |     |
  |                | SLOW GAME        |     |     | ON  |     |     |     |     |     |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | LOCK UP ON 7-8 | NO               |     |     |     | OFF |     |     |     |     |
  |  OF ANY KIND   | YES              |     |     |     | ON  |     |     |     |     |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | GRAPHICS TYPE  | REGULAR FRUIT    |     |     |     |     | OFF | OFF |     |     |
  |                | NUMBERS          |     |     |     |     | ON  | OFF |     |     |
  |                | CUSTOM OPTION 1  |     |     |     |     | OFF | ON  |     |     |
  |                | CUSTOM OPTION 2  |     |     |     |     | ON  | ON  |     |     |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | PAY TABLE      | 9322             |     |     |     |     |     |     | OFF | OFF |
  |                | 9323             |     |     |     |     |     |     | ON  | OFF |
  |                | 9324             |     |     |     |     |     |     | OFF | ON  |
  |                | 9321             |     |     |     |     |     |     | ON  | ON  |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+

  +-----------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIP SWITCHES BANK #2              |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | BONUS PAY %    | 1.0 %            | OFF | OFF | OFF |     |     |     |     |     |
  |                | 1.2 %            | ON  | OFF | OFF |     |     |     |     |     |
  |                | 3.3 %            | OFF | ON  | OFF |     |     |     |     |     |
  |                | 4.0 %            | ON  | ON  | OFF |     |     |     |     |     |
  |                | 5.0 %            | OFF | OFF | ON  |     |     |     |     |     |
  |                | 6.2 %            | ON  | OFF | ON  |     |     |     |     |     |
  |                | 8.0 %            | OFF | ON  | ON  |     |     |     |     |     |
  |                | 10.0 %           | ON  | ON  | ON  |     |     |     |     |     |
  +----------------+-----+------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | INTERNAL PROGRESSIVE | ON         |     |     |     | OFF |     |     |     |     |
  |                      | OFF        |     |     |     | ON  |     |     |     |     |
  +----------------+-----+------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | NOT USED       |                  |     |     |     |     | OFF | OFF | OFF | OFF |
  +----------------+------------------+-----+-----+-----+-----+-----+-----+-----+-----+

  +-----------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIP SWITCHES BANK #3              |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | MAXIMUM BET      | 8              | OFF | OFF |     |     |     |     |     |     |
  |                  | 16             | ON  | OFF |     |     |     |     |     |     |
  |                  | 32             | OFF | ON  |     |     |     |     |     |     |
  |                  | 64             | ON  | ON  |     |     |     |     |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | COIN IN-TIME OUT | OFF            |     |     | OFF |     |     |     |     |     |
  |                  | ON             |     |     | ON  |     |     |     |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DEMONATION       | $1.00          |     |     |     | OFF | OFF | OFF |     |     |
  |                  | 50 CENTS       |     |     |     | ON  | OFF | OFF |     |     |
  |                  | 25 CENTS       |     |     |     | OFF | ON  | OFF |     |     |
  |                  | 10 CENTS       |     |     |     | ON  | ON  | OFF |     |     |
  |                  | 5 CENTS        |     |     |     | OFF | OFF | ON  |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | PROGRESSIVE SIGN | OFF            |     |     |     |     |     |     | OFF | OFF |
  |                  | SINGLE LEVEL   |     |     |     |     |     |     | ON  | OFF |
  |                  | MULTI-LEVEL    |     |     |     |     |     |     | OFF | ON  |
  |                  | TEST MODE      |     |     |     |     |     |     | ON  | ON  |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+

  +-----------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIP SWITCHES BANK #4              |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | BUTTON LOCKOUT   | OFF            | OFF |     |     |     |     |     |     |     |
  |                  | ON             | ON  |     |     |     |     |     |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | LOCKOUT POLARITY | ACTIVE HIGH    |     | OFF |     |     |     |     |     |     |
  |                  | ACTIVE LOW     |     | ON  |     |     |     |     |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | PRINTER TYPE     | EPSON 276A     |     |     | OFF | OFF |     |     |     |     |
  |                  | STAR 300       |     |     | ON  | OFF |     |     |     |     |
  |                  | ITHACA 75      |     |     | OFF | ON  |     |     |     |     |
  |                  | CITIZEN 3541   |     |     | ON  | ON  |     |     |     |     |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | PAPER LOW SENSOR | OFF            |     |     |     |     | OFF | OFF | OFF | OFF |
  |                  | ON             |     |     |     |     | ON  | OFF | OFF | OFF |
  +------------------+----------------+-----+-----+-----+-----+-----+-----+-----+-----+


  NOTE: Set bill validator DIP switches for 1 pulse per dollar. When you choose the
        demonation with the DIP switch on the board, the software will know how many
        credits to give per dollar.


  Pay Table Diagram: (Set With DIP Switches Bank #1, numbers 7 & 8)


  --- 9323 ---

  3 Flaming Sevens    75 for 1
  3 Sevens            40 for 1
  9 Bars              15 for 1
  6 Bars              12 for 1
  3 Bars              10 for 1
  3 Cherries          08 for 1
  2 Cherries          05 for 1
  1 Cherries          02 for 1

  With 8 or more Bet:

  9 of any one = 1000 for 1
  8 of any one =  500 for 1
  7 of any one =  100 for 1

  If Cherry Bonus is on 5 or more Cherries, Pays Bonus.


  --- 9324 ---

  3 Flaming Sevens    75 for 1
  3 Sevens            40 for 1
  9 Bars              15 for 1
  6 Bars              12 for 1
  3 Bars              10 for 1
  3 Cherries          08 for 1
  2 Cherries          04 for 1
  1 Cherries          01 for 1

  With 8 or more Bet:

  9 of any one = 5000 for 1
  8 of any one = 2500 for 1
  7 of any one =  500 for 1

  If Cherry Bonus is on 5 or more Cherries, Pays Bonus.


  --- 9321 ---

  3 Flaming Sevens    75 for 1
  3 Sevens            40 for 1
  9 Bars              15 for 1
  6 Bars              12 for 1
  3 Bars              10 for 1
  3 Cherries          08 for 1
  2 Cherries          05 for 1
  1 Cherries          02 for 1

  With 8 or more Bet:

  9 of any one = 3000 for 1
  8 of any one = 1500 for 1
  7 of any one =  300 for 1

  If Cherry Bonus is on 5 or more Cherries, Pays Bonus.


  --- 9322 ---

  3 Flaming Sevens   100 for 1
  3 Sevens            50 for 1
  9 Bars              18 for 1
  6 Bars              14 for 1
  3 Bars              10 for 1
  3 Cherries          10 for 1
  2 Cherries          05 for 1
  1 Cherries          02 for 1

  With 8 or more Bet:

  9 of any one = 1000 for 1
  8 of any one =  500 for 1
  7 of any one =  100 for 1

  If Cherry Bonus is on 5 or more Cherries, Pays Bonus.

  -------------------------------------------------------------------------

        CYBERDYNE 72 PIN GAME LOGIC CONNECTOR

            Parts Side |  |  | Solder Side
        ---------------+--+--+---------------
             Red Video |01|37| Green Video
            Blue Video |02|38| Video Sync
            Speaker (+)|03|39| Speaker Ground
                       |04|40|
                       |05|41|
                       |06|42|
          Drop Door SW |07|43| Change SW
         Logic Door SW |08|44| Main Door SW
          Start Switch |09|45|
            Low Switch |10|46|
    Play Credit Switch |11|47|
     Take Score Switch |12|48|
      Double Up Switch |13|49|
           High Switch |14|50|
           Button Lock |15|51|
                       |16|52|
                       |17|53|
         (DBA) Inuput0 |18|54| Coin Switch "A"
       Coin Switch "B" |19|55|
   Book Keeping Switch |20|56| Stats / Setup SW
            Collect SW |21|57|
    Hopper Full Switch |22|58|
       Counter Out "A" |23|59|
                  Bell |24|60| Lockout mars (DBA)
            Tower Lamp |25|61|
       Counter Out "D" |26|62|
          Played Meter |27|63| Lack of Hopper Counter
  Count down Meter Out |28|64|
            Start Lamp |29|65|
              Low Lamp |30|66| Collect Lamp
      Play Credit Lamp |31|67| Progressive Link
       Take Score Lamp |32|68|
        Double Up Lamp |33|69| TXD
             High Lamp |34|70|
                       |35|71|
        Switch Grounds |36|72| Switch Grounds
                       +--+--+


           20 Pin Game PCB Power Connector

            Parts Side |  |  | Solder Side
        ---------------+--+--+---------------
          Logic Ground |01|11| Logic Ground
          Logic Ground |02|12| Logic Ground
                +5 VDC |03|13|
                +5 VDC |04|14| +5 VDC
               +12 VDC |05|15| +12 VDC
                       |06|16|
                       |07|17|
                       |08|18|
          Logic Ground |09|19| Logic Ground
          Logic Ground |10|20| Logic Ground
                       +--+--+

  -------------------------------------------------------------------------

  Cyberdyne Printer Harness (Flaming 7)
  -------------------------------------

                         +12V.----+----+
                                  |    |
                                  Z    Z
                                  Z    Z  470 ohm 1/4 watt resistors
                                  Z    Z
                                  |    |
  PCB Pin 69 Solder Side (TXD) ---+----|---- DB25 pin 3 (received data)
                                       |
                                       +---- DB25 pin 6

  PCB Pin 22 Parts Side -------------------- DB25 pins 1 & 7 (Single & Safety)

  PCB Pin 35 Parts Side -------------------- DB25 pin 20 (Printer Busy Signal)


  DIP Switches settings for Citizen Printer:

  Model # 3551
  DIP SW #1 = 5, 7, 9, 10 OFF.
  DIP SW #2 = 5, 7, 8, OFF.

  -------------------------------------------------------------------------

  Tech Notes:

  W-4 PCB type has some hacks...

  1) PPI 8255 pin 10 (PC7) is wired to pin 22 (PB4)...

  2) Pin 39 from solder side (speaker GND) and pin 04 from components side (PPI 8255 pin 11, PC6)
     are wired with a DS2401 (sillicon serial number) device, to pins 1 & 2 (GND & DATA).

     DS2401 has 64bits of internal data: 8 bits for family type + 48 bits for serial + 8 bits for CRC.

     Maxim/Dallas 1-Wire devices use what is called Dow CRC to check that the data bytes were received correctly.
     This returns an 8-bit CRC value as the last byte and uses a polynomial of X^8 + X^5 + X^4 + X^0.

  3) Z80 /INT line (pin 16) is out of socket and wired to a 74161.


  Custom Hardware Notes:

  To initialize the game the first time, enter BOOKS/STATS/SETUP (Key 0),
  and advance through the menus with RESET key (key R). When finish press
  again BOOKS/STATS/SETUP (Key 0) to exit the setup mode.

  ---

  GFX Bank 1 (chars and BG graphics) is colors-accurate.
  GFX Bank 2 needs some work...

  There is a window that should shows the bonus graphics.
  This graphic icon is taken from Bank 2, and is the "representative"
  for the chosen gfx set.

  Code writes the bonus graphics at 9A80-9A89, and the "reel scroll"
  at B0C0-B0FF...

  Tileset:
  Reels GFX 1: 00-00-01-02-03-04-05-06-07-00
  Reels GFX 2: 00-5A-5B-5C-5D-5E-5F-60-61-00
  Reels GFX 3: 9F-A0-A1-A2-A3-A4-A5-A6-A7-A8
  Reels GFX 4: E7-E8-E9-EA-EB-EC-ED-EE-EF-F0

  ...and written 0x10 at B0C0-B0FF.


  The taiwanese hardware needs better decoded reels graphics ROMs,
  The chars/text graphics are properly decodes, but texts strings
  look bad in the game.

*******************************************************************************************/

/*
  Flaming 7
  Cyberdyne Systems, Inc.

  W4 hardware.

  GFX sets:
  1) Red, White & Blue 7's
  2) Hollywood Nights.

  To boot the first time, press the BET 1 button (key M), and then BOOKS/STATS/SETUP (Key 0) twice.
  For further boots, just press BET 1 button. (the real hardware boots straight due to electronic
  serial number protection)

  To enter the Bookkeeping/Setup mode, press BOOKS/STATS/SETUP (key 0). Use RESET key (key R) to
  navigate between pages. Press BOOKS/STATS/SETUP (key 0) to exit the Bookkeeping mode.

*/
ROM_START( fl7_3121 )  // Red, White & Blue 7's + Hollywood Nights. Serial 7D063B800000.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "main.14b",  0x0000, 0x8000, CRC(5756e927) SHA1(5448e1ca4ae060b086145eee10b8dc6eb05acc56) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.7h",  0x00000, 0x8000, CRC(e2a2bf3e) SHA1(545ccd46f1fb65573778c3d14e73abb7d5f0ceda) )
	ROM_LOAD( "6.8h",  0x08000, 0x8000, CRC(46038515) SHA1(630b8fe920b9eded66705c39fbb8f2a49cae05a7) )
	ROM_LOAD( "7.10h", 0x10000, 0x8000, CRC(56c5c6e0) SHA1(9b64466899df17b5198249c12a2bdcd08383bb07) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.1h",  0x00000, 0x2000, CRC(22e23ce1) SHA1(5d1314cb259f6ab6d25abc8c019a74696d5ebc44) )
	ROM_LOAD( "2.3h",  0x02000, 0x2000, CRC(d3a097a5) SHA1(7100fb3570fc30aa7df4558a61cd165a67ae916d) )
	ROM_LOAD( "3.4h",  0x04000, 0x2000, CRC(c3207316) SHA1(3caeb0342aa23e6f3b5f1bf72957dce890ac6bc6) )
	ROM_LOAD( "4.5h",  0x06000, 0x2000, CRC(8d4a9198) SHA1(44fad32da4b018c1293320dc584e93d17ca71b2b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.g13", 0x0000, 0x0100, CRC(3ed8a612) SHA1(4189f1abb0033aeb64b56f63bdbc29d980c43909) )
	ROM_LOAD( "82s129.g14", 0x0100, 0x0100, CRC(aa068a22) SHA1(42c6d77e5aa360c529f8aca6b925010c15eedcd7) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "82s123.d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "82s129.f3", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "82s123.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial DS2401 */
	ROM_LOAD( "ds2401.bin", 0x0000, 0x0008, CRC(b7078792) SHA1(f9eba1587b65ed9bc07ea6c4b2d393fb43f60659) ) // Hand built to match our ROM set

ROM_END

/*
  Flaming 7's
  Cyberdyne Systems.

  Main - 50.
  Custom Hardware.

*/
ROM_START( fl7_50 )  // Serial 00000069A1C9.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "50-main.u22",  0x0000, 0x8000, CRC(e097e317) SHA1(a903144cc2290b7e22045490784b592adbf9ba97) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c1001.u6",  0x00000, 0x20000, CRC(00eac3c1) SHA1(1a955f8bc044e17f0885b4b126a66d7ad191e410) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "27c256.u3",   0x0000, 0x8000, CRC(cfc8f3e2) SHA1(7dd72e3ffb0904776f3c07635b953e72f4c63068) )

	/* Proper bipolar PROM dump */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u1", 0x0000, 0x0100, CRC(3fe7e369) SHA1(cf4ae287cb58581a4bf9e9ff1994426461fb38cc) )
	ROM_CONTINUE(           0x0000, 0x0100)  // palette data is stored in the second half.

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dummy", 0x0000, 0x0020, NO_DUMP )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial */
	ROM_LOAD( "eserial.bin", 0x0000, 0x0008, NO_DUMP ) // Hand built to match our ROM set

ROM_END


/*
  Flaming 7's
  Cyberdyne Systems.

  Main - 500.
  Custom Hardware.

*/
ROM_START( fl7_500 )  // Serial 000000125873.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "500-main.u22",  0x0000, 0x8000, CRC(e2c82c67) SHA1(951b0044de9b6104f51aa5a3176d0ea475415f7c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c1001.u6",  0x00000, 0x20000, CRC(00eac3c1) SHA1(1a955f8bc044e17f0885b4b126a66d7ad191e410) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "27c256.u3",   0x0000, 0x8000, CRC(4e3bd980) SHA1(202d3135da7ab435f487943079d88b170dc10955) )

	/* Proper bipolar PROM dump */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u1", 0x0000, 0x0100, CRC(3fe7e369) SHA1(cf4ae287cb58581a4bf9e9ff1994426461fb38cc) )
	ROM_CONTINUE(           0x0000, 0x0100)  // palette data is stored in the second half.

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dummy", 0x0000, 0x0020, NO_DUMP )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial */
	ROM_LOAD( "eserial.bin",  0x0000, 0x0008, NO_DUMP ) // Hand built to match our ROM set

ROM_END


/*
  Flaming 7's
  Cyberdyne Systems.

  Main - 2000.
  Custom Hardware.

*/
ROM_START( fl7_2000 )  // Serial 00000063A47F.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "2000_main_27c256.u22",  0x0000, 0x8000, CRC(9659b045) SHA1(801b6733b70b35de65cd8faba6814fa013c05ad0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "m27c1001.u6",  0x00000, 0x20000, CRC(5a2157bb) SHA1(2b170102caf1224df7a6d33bb84d19114f453d89) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "27c256.u3",   0x0000, 0x8000, CRC(cfc8f3e2) SHA1(7dd72e3ffb0904776f3c07635b953e72f4c63068) )

	/* Proper bipolar PROM dump */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u1", 0x0000, 0x0100, CRC(3fe7e369) SHA1(cf4ae287cb58581a4bf9e9ff1994426461fb38cc) )
	ROM_CONTINUE(           0x0000, 0x0100)  // palette data is stored in the second half.

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dummy", 0x0000, 0x0020, NO_DUMP )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial */
	ROM_LOAD( "eserial.bin", 0x0000, 0x0008, NO_DUMP ) // Hand built to match our ROM set

ROM_END


/*
  Flaming 7's
  Cyberdyne Systems.

  2000 Bonus. Egyptian Gold.
  Custom Hardware.

*/
ROM_START( fl7_2k16 )  // Serial 000000743111.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "2000_bonus_27c256.u22",  0x0000, 0x8000, CRC(290c37b0) SHA1(a90093d2139fd5ca261870298c2d84648f14e6bc) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "egyptian_gold_m27c1001.u6", 0x00000, 0x20000, CRC(5a2157bb) SHA1(2b170102caf1224df7a6d33bb84d19114f453d89) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "egyptian_gold_27c256.u3",   0x0000, 0x8000, CRC(11241dae) SHA1(7197813304623c022dc37aa603c0cb067c8bf1fd) )

	/* Bipolar PROM dump borrowed from main sets */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u1", 0x0000, 0x0100, CRC(3fe7e369) SHA1(cf4ae287cb58581a4bf9e9ff1994426461fb38cc) )
	ROM_CONTINUE(           0x0000, 0x0100)  // palette data is stored in the second half.

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dummy", 0x0000, 0x0020, NO_DUMP )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial */
	ROM_LOAD( "eserial.bin", 0x0000, 0x0008, NO_DUMP ) // Hand built to match our ROM set

ROM_END


/*
  Flaming 7's (unknown version)
  Taiwanese Hardware.

  Needs proper graphics ROM decryption and gfxdecode...

*/
ROM_START( fl7_tw )  // Serial 00000050E9B7.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_tw.u20",  0x0000, 0x10000, CRC(50927a1b) SHA1(2557069f497b23f13978294f3ac108229d9db544) )  // identical halves.

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "m27c1001_tw.u1", 0x00000, 0x20000, CRC(e6099723) SHA1(31e73a81166dd0d50d51ead38d348e36018d0698) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "27c256_tw.u3",   0x0000, 0x8000, CRC(7f163639) SHA1(607aa1e7d15423409bb2cd49895502dc2e4b3e46) )

	/* Bipolar PROM dump borrowed from main sets */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u1", 0x0000, 0x0100, CRC(3fe7e369) SHA1(cf4ae287cb58581a4bf9e9ff1994426461fb38cc) )
	ROM_CONTINUE(           0x0000, 0x0100)  // palette data is stored in the second half.

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dummy", 0x0000, 0x0020, NO_DUMP )

	ROM_REGION(0x8, "fl7w4_id", 0)     /* Electronic Serial */
	ROM_LOAD( "eserial.bin", 0x0000, 0x0008, NO_DUMP ) // Hand built to match our ROM set

ROM_END


/*********************************************************************************************************************/

void goldstar_state::init_goldstar()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		if ((A & 0x30) == 0)
			ROM[A] ^= 0x82;
		else
			ROM[A] ^= 0xcc;
	}
}

//  this block swapping is the same for chry10, chrygld and cb3
//  the underlying bitswaps / xors are different however
void cb3_state::do_blockswaps(uint8_t* ROM)
{
	int A;

	static const uint16_t cherry_swaptables[32] = {
		/* to align with goldstar */
		0x0800, 0x4000, 0x2800, 0x5800,
		0x1800, 0x3000, 0x6800, 0x7000,
		0x0000, 0x4800, 0x2000, 0x5000,
		0x1000, 0x7800, 0x6000, 0x3800,
		/* bit below, I'm not sure, no exact match, but only the first ones matter,
		   as the rest is just garbage */
		0xc000, 0xc800, 0xd000, 0xd800,
		0xe000, 0xe800, 0xf000, 0xf800,
		0x8000, 0x8800, 0x9000, 0x9800,
		0xa000, 0xa800, 0xb000, 0xb800,
	};

	std::vector<uint8_t> buffer(0x10000);
	memcpy(&buffer[0], ROM, 0x10000);

	// swap some 0x800 blocks around..
	for (A =0;A<32; A++)
	{
		memcpy(ROM + A * 0x800, &buffer[cherry_swaptables[A]], 0x800);
	}
}

void cb3_state::dump_to_file( uint8_t* ROM)
{
	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x10000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

uint8_t cb3_state::cb3_decrypt(uint8_t cipherText, uint16_t address)
{
	int idx;
	uint8_t output;
	int rotation[8] = {1, 0, 0, 1, 0, 1, 1, 1};
	int sbox[8] = {0x08, 0x08, 0x28, 0x00, 0x20, 0x20, 0x88, 0x88};

	idx = BIT(cipherText, 1) | (BIT(address,0) << 1) | (BIT(address, 4) << 2);

	if (rotation[idx] == 0)
		output = bitswap<8>(cipherText, 5, 6, 3, 4, 7, 2, 1, 0);   // rotates bit #3, #5 and #7 in one direction...
	else
		output = bitswap<8>(cipherText, 3, 6, 7, 4, 5, 2, 1, 0);   // ... or in the other

	return output ^ sbox[idx];
}

uint8_t cb3_state::chry10_decrypt(uint8_t cipherText)
{
	return cipherText ^ (BIT(cipherText, 4) << 3) ^ (BIT(cipherText, 1) << 5) ^ (BIT(cipherText, 6) << 7);
}

void cb3_state::init_chry10()
{
	uint8_t *ROM = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	for (int i = 0; i < size; i++)
	{
		ROM[i] = chry10_decrypt(ROM[i]);
	}

	do_blockswaps(ROM);

	/* The game has a PIC for protection.
	   If the code enter to this sub, just
	   keeps looping eternally...
	*/
	ROM[0xA5DC] = 0xc9;

	dump_to_file(ROM);
}

void cb3_state::init_cb3()
{
	uint8_t *ROM = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	for (int i = 0; i < size; i++)
	{
		ROM[i] = cb3_decrypt(ROM[i], i);
	}

	do_blockswaps(ROM);
	dump_to_file(ROM);
}


void cb3_state::init_chrygld()
{
	uint8_t *ROM = memregion("maincpu")->base();
	do_blockswaps(ROM);

	// a data bitswap
	for (int A = 0; A < 0x10000; A++)
	{
		uint8_t dat = ROM[A];
		dat =  bitswap<8>(dat, 5, 6, 3, 4, 7, 2, 1, 0);
		ROM[A] = dat;
	}

	dump_to_file(ROM);
}

void cmaster_state::init_cm()
{
	uint8_t *ROM = memregion("maincpu")->base();

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0021] = 0x9b;
	ROM[0x0025] = 0x9b;
}

void cmaster_state::init_cmv4()
{
	uint8_t *ROM = memregion("maincpu")->base();

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0209] = 0x9b;
	ROM[0x020d] = 0x9b;
}

void goldstar_state::init_cmast91()
{
	uint8_t *ROM = memregion("maincpu")->base();

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0070] = 0x9b;
	ROM[0x0a92] = 0x9b;
}

void wingco_state::init_lucky8a()
{
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x0010] = 0x21;
}

void cmaster_state::init_nfb96sea()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];
		switch (i & 7)
		{
			case 0: x = bitswap<8>(x ^ 0x80, 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 1: x = bitswap<8>(x ^ 0xa0, 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 2: x = bitswap<8>(x ^ 0x02, 5, 6, 3, 4, 1, 2, 7, 0); break;
			case 3: x = bitswap<8>(x ^ 0xa0, 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 4: x = bitswap<8>(x ^ 0x82, 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 5: x = bitswap<8>(x ^ 0x02, 1, 6, 7, 4, 5, 2, 3, 0); break;
			case 6: x = bitswap<8>(x ^ 0x08, 3, 6, 1, 4, 7, 2, 5, 0); break;
			case 7: x = bitswap<8>(x ^ 0x80, 5, 6, 3, 4, 1, 2, 7, 0); break;
		}

		ROM[i] = x;
	}
}


void cmaster_state::init_schery97()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];
		switch (i & 0x12)
		{
			case 0x00: x = bitswap<8>(x ^ 0x3e, 1, 0, 7, 6, 5, 4, 3, 2); break;
			case 0x02: x = bitswap<8>(x ^ 0x4d, 0, 7, 6, 5, 4, 3, 2, 1); break;
			case 0x10: x = bitswap<8>(x ^ 0x24, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 0x12: x = bitswap<8>(x ^ 0xbb, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}

		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x1d, 0x1d, read8_delegate(FUNC(cmaster_state::fixedvala8_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x2a, 0x2a, read8_delegate(FUNC(cmaster_state::fixedvalb4_r), this));
	/* Oki 6295 at 0x20 */
}

void cmaster_state::init_schery97a()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];
		switch (i & 6)
		{
			case 0: x = bitswap<8>(x ^ 0xb9, 4, 0, 6, 7, 3, 1, 5, 2); break;
			case 2: x = bitswap<8>(x ^ 0x8f, 6, 7, 4, 0, 3, 2, 1, 5); break;
			case 4: x = bitswap<8>(x ^ 0xd2, 3, 4, 0, 2, 5, 6, 1, 7); break;
			case 6: x = bitswap<8>(x ^ 0xd1, 6, 0, 2, 1, 4, 5, 3, 7); break;
		}

		ROM[i] = x;
	}


	m_maincpu->space(AS_IO).install_read_handler(0x16, 0x16, read8_delegate(FUNC(cmaster_state::fixedval38_r), this));
	/* Oki 6295 at 0x20 */
}

void cmaster_state::init_skill98()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];
		switch (i & 0x12)
		{
			case 0x00: x = bitswap<8>(x ^ 0x21, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 0x02: x = bitswap<8>(x ^ 0x45, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 0x10: x = bitswap<8>(x ^ 0x23, 4, 3, 2, 1, 0, 7, 6, 5); break;
			case 0x12: x = bitswap<8>(x ^ 0x5b, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}

		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x1e, 0x1e, read8_delegate(FUNC(cmaster_state::fixedvalea_r), this));
	/* Oki 6295 at 0x20 */
}

void cmaster_state::init_nfb96_c1()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0x12)
		{
			case 0x00: x = bitswap<8>(x ^ 0xf5, 6, 4, 3, 7, 0, 1, 5, 2); break;
			case 0x02: x = bitswap<8>(x ^ 0xe6, 4, 6, 3, 0, 7, 2, 1, 5); break;
			case 0x10: x = bitswap<8>(x ^ 0x34, 0, 3, 5, 2, 4, 6, 1, 7); break;
			case 0x12: x = bitswap<8>(x ^ 0xc6, 2, 0, 4, 1, 6, 5, 3, 7); break;
		}
		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x31, 0x31, read8_delegate(FUNC(cmaster_state::fixedval68_r), this));

}

void cmaster_state::init_nfb96_c2()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0x22)
		{
			case 0x00: x = bitswap<8>(x ^ 0x5f, 6, 4, 3, 7, 0, 5, 2, 1); break;
			case 0x02: x = bitswap<8>(x ^ 0xe7, 4, 6, 3, 0, 7, 5, 1, 2); break;
			case 0x20: x = bitswap<8>(x ^ 0x18, 0, 3, 5, 2, 4, 7, 1, 6); break;
			case 0x22: x = bitswap<8>(x ^ 0x74, 2, 0, 4, 1, 6, 7, 3, 5); break;
		}

		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x21, 0x21, read8_delegate(FUNC(cmaster_state::fixedval58_r), this));
}

void cmaster_state::init_nfb96_d()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 5)
		{
			case 0: x = bitswap<8>(x ^ 0x6a, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 1: x = bitswap<8>(x ^ 0xcc, 0, 7, 6, 5, 4, 3, 2, 1); break;
			case 4: x = bitswap<8>(x ^ 0x8f, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 5: x = bitswap<8>(x ^ 0x93, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}
		ROM[i] = x;
	}
	// nfb96b needs both of these
	m_maincpu->space(AS_IO).install_read_handler(0x23, 0x23, read8_delegate(FUNC(cmaster_state::fixedval80_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x5a, 0x5a, read8_delegate(FUNC(cmaster_state::fixedvalaa_r), this));

	// csel96b
	m_maincpu->space(AS_IO).install_read_handler(0x6e, 0x6e, read8_delegate(FUNC(cmaster_state::fixedval96_r), this));

}


void cmaster_state::init_nfb96_dk()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 5)
		{
			case 0: x = bitswap<8>(x ^ 0xce, 1, 0, 7, 6, 5, 4, 3, 2); break;
			case 1: x = bitswap<8>(x ^ 0x9e, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 4: x = bitswap<8>(x ^ 0xc3, 0, 7, 6, 5, 4, 3, 2, 1); break;
			case 5: x = bitswap<8>(x ^ 0xdb, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}
		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x2e, 0x2e, read8_delegate(FUNC(cmaster_state::fixedvalbe_r), this));

}

void cmaster_state::init_rp35()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 3)
		{
			case 0: x = bitswap<8>(x ^ 0x2a, 0, 7, 6, 5, 4, 3, 2, 1); break;
			case 1: x = bitswap<8>(x ^ 0x1c, 4, 3, 2, 1, 0, 7, 6, 5); break;
			case 2: x = bitswap<8>(x ^ 0x4f, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 3: x = bitswap<8>(x ^ 0x23, 1, 0, 7, 6, 5, 4, 3, 2); break;
		}
		ROM[i] = x;
	}

	m_maincpu->space(AS_IO).install_read_handler(0x5e, 0x5e, read8_delegate(FUNC(cmaster_state::fixedval84_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x36, 0x36, read8_delegate(FUNC(cmaster_state::fixedval90_r), this));
}

void cmaster_state::init_rp36()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 5)
		{
			case 0: x = bitswap<8>(x ^ 0xee, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 1: x = bitswap<8>(x ^ 0x9f, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 4: x = bitswap<8>(x ^ 0xc7, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 5: x = bitswap<8>(x ^ 0xc3, 3, 2, 1, 0, 7, 6, 5, 4); break;
		}

		ROM[i] = x;
	}

	m_maincpu->space(AS_IO).install_read_handler(0x34, 0x34, read8_delegate(FUNC(cmaster_state::fixedvalb2_r), this));
}

void cmaster_state::init_rp36c3()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0xa)
		{
			case 0x0: x = bitswap<8>(x ^ 0xfd, 6, 4, 0, 7, 3, 1, 5, 2); break;
			case 0x2: x = bitswap<8>(x ^ 0xee, 4, 6, 7, 0, 3, 2, 1, 5); break;
			case 0x8: x = bitswap<8>(x ^ 0x2c, 0, 3, 4, 2, 5, 6, 1, 7); break;
			case 0xa: x = bitswap<8>(x ^ 0xd6, 2, 0, 6, 1, 4, 5, 3, 7); break;
		}

		ROM[i] = x;
	}

	m_maincpu->space(AS_IO).install_read_handler(0x17, 0x17, read8_delegate(FUNC(cmaster_state::fixedval48_r), this));
}

void cmaster_state::init_rp96sub()  // 95 33 95 33 70 6C 70 6C... XORs seem ok. need bitswap and handler.
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 5)
		{
			case 0: x = bitswap<8>(x ^ 0x6a, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 1: x = bitswap<8>(x ^ 0xcc, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 4: x = bitswap<8>(x ^ 0x8f, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 5: x = bitswap<8>(x ^ 0x93, 7, 6, 5, 4, 3, 2, 1, 0); break;
		}

		ROM[i] = x;
	}

//  m_maincpu->space(AS_IO).install_read_handler(0x34, 0x34, read8_delegate(FUNC(cmaster_state::fixedvalb2_r),this));
}


void cmaster_state::init_po33()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0x14)
		{
			case 0x00: x = bitswap<8>(x ^ 0xde, 2, 1, 0, 7, 6, 5, 4, 3); break;
			case 0x04: x = bitswap<8>(x ^ 0x3c, 0, 7, 6, 5, 4, 3, 2, 1); break;
			case 0x10: x = bitswap<8>(x ^ 0x2f, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 0x14: x = bitswap<8>(x ^ 0x5b, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}

		ROM[i] = x;
	}
	m_maincpu->space(AS_IO).install_read_handler(0x32, 0x32, read8_delegate(FUNC(cmaster_state::fixedval74_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x12, 0x12, read8_delegate(FUNC(cmaster_state::fixedval09_r), this));
	/* oki6295 at 0x20 */
}

void cmaster_state::init_match133()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t x = ROM[i];

		switch (i & 0x12)
		{
			case 0x00: x = bitswap<8>(x ^ 0xde, 3, 2, 1, 0, 7, 6, 5, 4); break;
			case 0x02: x = bitswap<8>(x ^ 0x3d, 1, 0, 7, 6, 5, 4, 3, 2); break;
			case 0x10: x = bitswap<8>(x ^ 0x2f, 4, 3, 2, 1, 0, 7, 6, 5); break;
			case 0x12: x = bitswap<8>(x ^ 0x5c, 4, 3, 2, 1, 0, 7, 6, 5); break;
		}

		ROM[i] = x;
	}

	m_maincpu->space(AS_IO).install_read_handler(0x16, 0x16, read8_delegate(FUNC(cmaster_state::fixedvalc7_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x1a, 0x1a, read8_delegate(FUNC(cmaster_state::fixedvale4_r), this));
}

void cb3_state::init_cherrys()
{
	uint8_t *ROM = memregion("maincpu")->base();
	unsigned char rawData[256] = {
		0xCC, 0xCD, 0xCE, 0xCF, 0xC8, 0xC9, 0xCA, 0xCB, 0xC4, 0xC5, 0xC6, 0xC7,
		0xC0, 0xC1, 0xC2, 0xC3, 0xDC, 0xDD, 0xDE, 0xDF, 0xD8, 0xD9, 0xDA, 0xDB,
		0xD4, 0xD5, 0xD6, 0xD7, 0xD0, 0xD1, 0xD2, 0xD3, 0xEC, 0xED, 0xEE, 0xEF,
		0xE8, 0xE9, 0xEA, 0xEB, 0xE4, 0xE5, 0xE6, 0xE7, 0xE0, 0xE1, 0xE2, 0xE3,
		0xFC, 0xFD, 0xFE, 0xFF, 0xF8, 0xF9, 0xFA, 0xFB, 0xF4, 0xF5, 0xF6, 0xF7,
		0xF0, 0xF1, 0xF2, 0xF3, 0x8C, 0x8D, 0x8E, 0x8F, 0x88, 0x89, 0x8A, 0x8B,
		0x84, 0x85, 0x86, 0x87, 0x80, 0x81, 0x82, 0x83, 0x9C, 0x9D, 0x9E, 0x9F,
		0x98, 0x99, 0x9A, 0x9B, 0x94, 0x95, 0x96, 0x97, 0x90, 0x91, 0x92, 0x93,
		0xAC, 0xAD, 0xAE, 0xAF, 0xA8, 0xA9, 0xAA, 0xAB, 0xA4, 0xA5, 0xA6, 0xA7,
		0xA0, 0xA1, 0xA2, 0xA3, 0xBC, 0xBD, 0xBE, 0xBF, 0xB8, 0xB9, 0xBA, 0xBB,
		0xB4, 0xB5, 0xB6, 0xB7, 0xB0, 0xB1, 0xB2, 0xB3, 0x4C, 0x4D, 0x4E, 0x4F,
		0x48, 0x49, 0x4A, 0x4B, 0x44, 0x45, 0x46, 0x47, 0x40, 0x41, 0x42, 0x43,
		0x5C, 0x5D, 0x5E, 0x5F, 0x58, 0x59, 0x5A, 0x5B, 0x54, 0x55, 0x56, 0x57,
		0x50, 0x51, 0x52, 0x53, 0x6C, 0x6D, 0x6E, 0x6F, 0x68, 0x69, 0x6A, 0x6B,
		0x64, 0x65, 0x66, 0x67, 0x60, 0x61, 0x62, 0x63, 0x7C, 0x7D, 0x7E, 0x7F,
		0x78, 0x79, 0x7A, 0x7B, 0x74, 0x75, 0x76, 0x77, 0x70, 0x71, 0x72, 0x73,
		0x0C, 0x0D, 0x0E, 0x0F, 0x08, 0x09, 0x0A, 0x0B, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x1C, 0x1D, 0x1E, 0x1F, 0x18, 0x19, 0x1A, 0x1B,
		0x14, 0x15, 0x16, 0x17, 0x10, 0x11, 0x12, 0x13, 0x2C, 0x2D, 0x2E, 0x2F,
		0x28, 0x29, 0x2A, 0x2B, 0x24, 0x25, 0x26, 0x27, 0x20, 0x21, 0x22, 0x23,
		0x3C, 0x3D, 0x3E, 0x3F, 0x38, 0x39, 0x3A, 0x3B, 0x34, 0x35, 0x36, 0x37,
		0x30, 0x31, 0x32, 0x33
	};

	for (int i = 0; i < 0x10000; i++)
	{
		ROM[i] = ROM[i] ^ rawData[i & 0xff];
	}

}

/* todo: remove these patches! */
void unkch_state::init_unkch1()
{
	// game stores $02 at ($D75C) and expects it to change
	// possibly expecting stack to grow to this point in NMI handler?
	// it does this before enabling vblank irq, so if that's the case there's a missing nmi source
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x9d52] = 0x00;
	ROM[0x9d53] = 0x00;
}

void unkch_state::init_unkch3()
{
	// game stores $04 at ($D77F) and expects it to change
	// possibly expecting stack to grow to this point in NMI handler?
	// it does this before enabling vblank irq, so if that's the case there's a missing nmi source
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x9b86] = 0x00;
	ROM[0x9b87] = 0x00;
}

void unkch_state::init_unkch4()
{
	// game stores $02 at ($D75C) and expects it to change
	// possibly expecting stack to grow to this point in NMI handler?
	// it does this before enabling vblank irq, so if that's the case there's a missing nmi source
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x9a6e] = 0x00;
	ROM[0x9a6f] = 0x00;
}

void cmaster_state::init_tonypok()
{
	// the ppi doesn't seem to work properly, so just install the inputs directly
	address_space &io = m_maincpu->space(AS_IO);
	io.install_read_port(0x04, 0x04, "IN0" );
	io.install_read_port(0x05, 0x05, "IN1" );
	io.install_read_port(0x06, 0x06, "IN2" );

}

void goldstar_state::init_super9()
{
	uint8_t *src = memregion("gfx1")->base();
	for (int i = 0; i < 0x20000; i++)
	{
//      src[i] = bitswap<8>(src[i], 7, 4, 2, 1, 6, 5, 3, 0);
//      src[i] = bitswap<8>(src[i], 7, 3, 2, 6, 1, 5, 4, 0);
		src[i] = bitswap<8>(src[i], 7, 3, 2, 6, 5, 1, 4, 0);
	}

	uint8_t *src2 = memregion("gfx2")->base();
	for (int i = 0; i < 0x8000; i++)
	{
//      src2[i] = bitswap<8>(src2[i], 7, 4, 2, 1, 6, 5, 3, 0);
//      src2[i] = bitswap<8>(src2[i], 7, 3, 2, 6, 1, 5, 4, 0);
		src2[i] = bitswap<8>(src2[i], 3, 7, 6, 2, 5, 1, 0, 4);   // endianess
	}

}

void cb3_state::init_cb3e()
{
/*  program bitswap */
	uint8_t *ROM = memregion("maincpu")->base();
	do_blockswaps(ROM);

	for (int i = 0; i < 0x10000; i++)
	{
		uint8_t dat = ROM[i];
		dat =  bitswap<8>(dat, 5, 6, 3, 4, 7, 2, 1, 0);
		ROM[i] = dat;
	}

/*  bank 1 graphics */
	uint8_t *src = memregion("gfx1")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		src[i] = bitswap<8>(src[i], 4, 3, 2, 5, 1, 6, 0, 7);      // OK
	}

/*  bank 2 graphics */
	uint8_t *src2 = memregion("gfx2")->base();
	for (int i = 0; i < 0x8000; i++)
	{
		src2[i] = bitswap<8>(src2[i], 3, 4, 2, 5, 1, 6, 0, 7);    // OK
	}
}

void goldstar_state::init_wcherry()
{
/*  bank 1 graphics */
	uint8_t *src = memregion("gfx1")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		src[i] = bitswap<8>(src[i], 4, 3, 2, 5, 1, 6, 0, 7);      // OK
	}

/*  bank 2 graphics */
	uint8_t *src2 = memregion("gfx2")->base();
	for (int i = 0; i < 0x8000; i++)
	{
		src2[i] = bitswap<8>(src2[i], 3, 4, 2, 5, 1, 6, 0, 7);    // OK
	}
}

/*
  Flaming 7's
  Cyberdyne Systems.

  Original custom hardware graphics decryption.

*/
void wingco_state::init_flaming7()
{
/*  bank 1 graphics */
	uint8_t *src = memregion("gfx1")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		src[i] = bitswap<8>(src[i], 4, 3, 2, 5, 1, 6, 0, 7);      // OK
	}

/*  bank 2 graphics */
	uint8_t *src2 = memregion("gfx2")->base();
	for (int i = 0; i < 0x8000; i++)
	{
		src2[i] = bitswap<8>(src2[i], 3, 4, 2, 5, 1, 6, 0, 7);    // OK
	}

/*  Patch to bypass the iButton electronic serial number check.

    5D79: jumps to $5D83 for the demo game.
          just patched to jump to $5D80 (jmp $6115)
          where the check is succesfull.
*/
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x5d7a] = 0x05;
}

/*
  Flaming 7's
  Cyberdyne Systems.

  Taiwanese custom hardware.

*/
void wingco_state::init_flam7_tw()
{
/*  Patch to bypass the iButton electronic serial number check.

    60B2: After last compare, code jumps to $60BC for the demo game.
          Just patched to jump to $60B9 (jmp $644E) where the check
          is successful.
*/
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x60b3] = 0x05;

}

void cmaster_state::init_tcl()
{
	/* only the first part is decrypted (and verified)*/

	uint8_t *dest = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();
	std::vector<uint8_t> src(len);

	int idx=0;
	memcpy(&src[0], dest, len);

	auto const rol =
		[] (int src, int n)
		{
			return bitswap<8>(src,(7+8-n)&7,(6+8-n)&7,(5+8-n)&7,(4+8-n)&7,(3+8-n)&7,(2+8-n)&7,(1+8-n)&7,(0+8-n)&7);
		};

	auto const writedest =
		[dest, &idx] (int n)
		{
			dest[idx]=n;
			dest[idx+0x10000]=n^0xff;
			idx++;
		};

	for (int i = 0; i < 64*1024; i += 4)
	{
		if (i & 0x8000)
		{
			writedest(rol(src[idx]^0x44,4)); // abcdefgh -> aFghaBcd
			writedest(rol(src[idx]^0x44,7)); // abcdefgh -> haBcdeFg
			writedest(rol(src[idx]^0x44,2)); // abcdefgh -> cdeFghaB
			writedest((src[idx]^0x44)^0xf0); // abcdefgh -> AbCEeFgh
		}
		else
		{
			writedest(rol(src[idx]^0x11,4)); // abcdefgh -> efgHabcD
			writedest(rol(src[idx]^0x11,7)); // abcdefgh -> HabcDefg
			writedest(rol(src[idx]^0x11,2)); // abcdefgh -> cDefgHab
			writedest((src[idx]^0x11)^0xf0); // abcdefgh -> ABCdefgH
		}
	}
}

void cmaster_state::init_super7()
{
	uint8_t *ROM = memregion("maincpu")->base();

	for (int A = 0; A < 0x8000; A++)
	{
		ROM[A] = (ROM[A] << 1) | BIT(ROM[A], 7);
	}
}

void wingco_state::init_luckylad()
{
	uint8_t *ROM = memregion("maincpu")->base();

	for (int i = 0x0000; i < 0x8000; i++)
	{
		uint8_t x = ROM[i];
		switch (i & 0x1111) // preliminary, text in 0x3ab0-0x3c70, 0x4c00-0x5160 ranges seems mostly ok, seems similar to the one used in luckygrl (jangou.cpp).
		{
			case 0x0000: x = bitswap<8>(x ^ 0x00, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0001: x = bitswap<8>(x ^ 0xa0, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0010: x = bitswap<8>(x ^ 0x88, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0011: x = bitswap<8>(x ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0100: x = bitswap<8>(x ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0101: x = bitswap<8>(x ^ 0x20, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0110: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: x = bitswap<8>(x ^ 0x88, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1000: x = bitswap<8>(x ^ 0x20, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1001: x = bitswap<8>(x ^ 0x00, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x1010: x = bitswap<8>(x ^ 0xa0, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1011: x = bitswap<8>(x ^ 0x80, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1100: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x1101: x = bitswap<8>(x ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x1110: x = bitswap<8>(x ^ 0x80, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1111: x = bitswap<8>(x ^ 0x00, 7, 6, 5, 4, 3, 2, 1, 0); break;
		}

		ROM[i] = x;
	}
}

/*********************************************
*                Game Drivers                *
**********************************************

       YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT       ROT    COMPANY              FULLNAME                                      FLAGS              LAYOUT */
GAMEL( 199?, goldstar,  0,        goldstar, goldstar, goldstar_state, init_goldstar,  ROT0, "IGS",               "Golden Star",                                 0,                 layout_goldstar )
GAMEL( 199?, goldstbl,  goldstar, goldstbl, goldstar, goldstar_state, empty_init,     ROT0, "IGS",               "Golden Star (Blue version)",                  0,                 layout_goldstar )
GAME(  199?, moonlght,  goldstar, moonlght, goldstar, goldstar_state, empty_init,     ROT0, "bootleg",           "Moon Light (v.0629, low program)",            0 )
GAME(  199?, moonlghta, goldstar, moonlght, goldstar, goldstar_state, empty_init,     ROT0, "bootleg",           "Moon Light (v.0629, high program)",           0 )
GAME(  199?, moonlghtb, goldstar, moonlght, goldstar, goldstar_state, empty_init,     ROT0, "bootleg",           "Moon Light (v.02L0A, low program)",           MACHINE_IMPERFECT_COLORS )  // need to check the odd palette value at 0xc780. should be black.
GAME(  199?, moonlghtc, goldstar, moonlght, goldstar, goldstar_state, empty_init,     ROT0, "bootleg",           "Moon Light (v.02L0A, high program, alt gfx)", MACHINE_IMPERFECT_COLORS )  // need to check the odd palette value at 0xc780. should be black.
GAMEL( 199?, chrygld,   0,        chrygld,  chrygld,  cb3_state,      init_chrygld,   ROT0, "bootleg",           "Cherry Gold I (set 1)",                       0,                 layout_chrygld )
GAMEL( 199?, chry10,    0,        chrygld,  chry10,   cb3_state,      init_chry10,    ROT0, "bootleg",           "Cherry 10 (bootleg with PIC16F84)",           0,                 layout_chrygld )
GAME(  199?, goldfrui,  goldstar, goldfrui, goldstar, goldstar_state, empty_init,     ROT0, "bootleg",           "Gold Fruit",                                  0 )                  // maybe fullname should be 'Gold Fruit (main 40%)'
GAME(  2001, super9,    goldstar, super9,   goldstar, goldstar_state, init_super9,    ROT0, "Playmark",          "Super Nove (Playmark)",                       MACHINE_NOT_WORKING )   // need to decode gfx and see the program loops/reset...
GAME(  2001, wcherry,   0,        wcherry,  chrygld,  goldstar_state, init_wcherry,   ROT0, "bootleg",           "Win Cherry (ver 0.16 - 19990219)",            MACHINE_NOT_WORKING )
GAME(  199?, star100,   0,        star100,  star100,  sanghopm_state, empty_init,     ROT0, "Sang Ho",           "Ming Xing 100 (Star 100)",                    MACHINE_IMPERFECT_COLORS )

// are these really dyna, or bootlegs?
GAMEL( 199?, ncb3,      0,        ncb3,     ncb3,     cb3_state,      empty_init,     ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 1)",          0,                 layout_cherryb3 )
GAMEL( 199?, cb3a,      ncb3,     ncb3,     cb3a,     cb3_state,      empty_init,     ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 2)",          0,                 layout_cherryb3 )
GAMEL( 199?, cb3,       ncb3,     ncb3,     ncb3,     cb3_state,      init_cb3,       ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, encrypted)",      0,                 layout_cherryb3 )
GAMEL( 199?, cb3b,      ncb3,     cherrys,  ncb3,     cb3_state,      init_cherrys,   ROT0, "Dyna",              "Cherry Bonus III (alt)",                      0,                 layout_cherryb3 )
GAME(  199?, cb3c,      ncb3,     cb3c,     chrygld,  cb3_state,      init_cb3,       ROT0, "bootleg",           "Cherry Bonus III (alt, set 2)",               MACHINE_NOT_WORKING)
GAMEL( 199?, cb3d,      ncb3,     ncb3,     ncb3,     cb3_state,      empty_init,     ROT0, "bootleg",           "Cherry Bonus III (set 3)",                    0,                 layout_cherryb3 )
GAMEL( 199?, cb3e,      ncb3,     cb3e,     chrygld,  cb3_state,      init_cb3e,      ROT0, "bootleg",           "Cherry Bonus III (set 4, encrypted bootleg)", 0,                 layout_chrygld )
GAMEL( 199?, chryglda,  ncb3,     cb3e,     chrygld,  cb3_state,      init_cb3e,      ROT0, "bootleg",           "Cherry Gold I (set 2, encrypted bootleg)",    0,                 layout_chrygld )  // Runs in CB3e hardware.

GAME(  1996, cmast97,   ncb3,     cm97,     chrygld,  cb3_state,      empty_init,     ROT0, "Dyna",              "Cherry Master '97",                           MACHINE_NOT_WORKING) // fix prom decode

// looks like a hack of Cherry Bonus 3
GAME(  199?, chryangl,  ncb3,     cm,       chryangl, cmaster_state,  init_cmv4,      ROT0, "<unknown>",         "Cherry Angel",                                MACHINE_NOT_WORKING )


// cherry master hardware has a rather different mem map, but is basically the same
GAMEL( 198?, cmv801,    0,        cm,       cmv801,   cmaster_state,  init_cm,        ROT0, "Corsica",           "Cherry Master (Corsica, ver.8.01)",           0,                 layout_cmv4 ) /* says ED-96 where the manufacturer is on some games.. */



// most of these are almost certainly bootlegs, with added features, hacked payouts etc. identifying which are
// the original, unmodified dyna versions is almost impossible due to lack of documentation from back in the day,
// even original boards almost always run modified sets
GAMEL( 1992, cmv4,      0,        cm,       cmv4,     cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master (ver.4, set 1)",                0,                 layout_cmv4 )
GAMEL( 1992, cmv4a,     cmv4,     cm,       cmv4,     cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master (ver.4, set 2)",                MACHINE_NOT_WORKING,  layout_cmv4 ) // stealth game?
GAMEL( 199?, cmwm,      cmv4,     cm,       cmv4,     cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master (Watermelon bootleg / hack)",   0,                 layout_cmv4 ) // CM Fruit Bonus ver.2 T bootleg/hack
GAMEL( 1995, cmfun,     cmv4,     cm,       cmv4,     cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master (Fun USA v2.5 bootleg / hack)", 0,                 layout_cmv4 )
GAMEL( 1991, cmaster,   0,        cm,       cmaster,  cmaster_state,  empty_init,     ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 1)",           0,                 layout_cmaster )
GAMEL( 1991, cmasterb,  cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 2)",           0,                 layout_cmasterb )
GAMEL( 1991, cmezspin,  cmaster,  cm,       cmezspin, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (E-Z Spin bootleg / hack)",   0,                 layout_cmezspin ) // CM Fruit Bonus 55 ver.2 bootleg/hack
GAMEL( 1991, cmasterc,  cmaster,  cmasterc, cmasterc, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 3)",           0,                 layout_cmasterc )
GAMEL( 1991, cmasterbv, cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 4, with Blitz Poker ROM?)", MACHINE_NOT_WORKING, layout_cmasterb ) // Cherry Master works, but no idea how to use the Blitz ROM
GAMEL( 1991, cmasterd,  cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 5)",           0,                 layout_cmasterb )
GAMEL( 1991, cmastere,  cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 6)",           0,                 layout_cmasterb )
GAMEL( 1991, cmasterf,  cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 7)",           0,                 layout_cmasterb )
GAMEL( 1991, cmasterg,  cmaster,  cm,       cmasterg, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 8, V4-B-)",    0,                 layout_cmasterb )
GAMEL( 1991, cmasterh,  cmaster,  cm,       cmasterb, cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.10)",                  0,                 layout_cmasterb )
GAMEL( 199?, super7,    cmaster,  cm,       cmaster,  cmaster_state,  init_super7,    ROT0, "bootleg",           "Super Seven",                                 MACHINE_NOT_WORKING, layout_cmasterb )

GAMEL( 1991, tonypok,   0,        cm,       tonypok,  cmaster_state,  init_tonypok,   ROT0, "Corsica",           "Poker Master (Tony-Poker V3.A, hack?)",       0 ,                layout_tonypok )
GAME(  199?, jkrmast,   0,        pkrmast,  pkrmast,  goldstar_state, empty_init,     ROT0, "<unknown>",         "Joker Master (V515)",                         MACHINE_NOT_WORKING ) // encrypted
GAME(  199?, jkrmasta,  jkrmast,  pkrmast,  pkrmast,  goldstar_state, empty_init,     ROT0, "<unknown>",         "Joker Master (V512)",                         MACHINE_NOT_WORKING ) // encrypted
GAME(  199?, pkrmast,   jkrmast,  pkrmast,  pkrmast,  goldstar_state, empty_init,     ROT0, "<unknown>",         "Poker Master (ED-1993 set 1)",                MACHINE_NOT_WORKING ) // incomplete dump + encrypted?
GAME(  1993, pkrmasta,  jkrmast,  pkrmast,  pkrmast,  goldstar_state, empty_init,     ROT0, "<unknown>",         "Poker Master (ED-1993 set 2)",                MACHINE_NOT_WORKING ) // incomplete dump + encrypted?


GAME(  1991, cmast91,   0,        cmast91,  cmast91,  goldstar_state, init_cmast91,   ROT0, "Dyna",              "Cherry Master '91 (ver.1.30)",                0 )
GAME(  1992, cmast92,   0,        cmast91,  cmast91,  goldstar_state, init_cmast91,   ROT0, "Dyna",              "Cherry Master '92",                           MACHINE_NOT_WORKING ) // no gfx roms are dumped
GAME(  1999, cmast99,   0,        cm,       cmast99,  cmaster_state,  init_cmv4,      ROT0, "Dyna",              "Cherry Master '99 (V9B.00)",                  MACHINE_NOT_WORKING )
GAME(  1999, cmast99b,  cmast99,  cm,       cmast99,  cmaster_state,  init_cmv4,      ROT0, "bootleg",           "Cherry Master '99 (V9B.00 bootleg / hack)",   MACHINE_NOT_WORKING )
GAME(  1993, aplan,     0,        cm,       cmast99,  cmaster_state,  init_cmv4,      ROT0, "WeaShing H.K.",     "A-Plan",                                      MACHINE_NOT_WORKING )

GAME(  1995, tcl,       0,        cm,       cmaster,  cmaster_state,  init_tcl,       ROT0, "Uniwang",           "Taiwan Chess Legend",                         MACHINE_NOT_WORKING ) // incomplete decryption

// --- Wing W-4 hardware ---
GAMEL( 1989, lucky8,    0,        lucky8,   lucky8,   wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 1, W-4)",                           0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 1989, lucky8a,   lucky8,   lucky8,   lucky8a,  wingco_state,   init_lucky8a,   ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 2, W-4)",                           0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 1989, lucky8b,   lucky8,   lucky8,   lucky8b,  wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 3, W-4, extended gfx)",             0,                     layout_lucky8p1 )  // only 1 control set...
GAMEL( 1989, lucky8c,   lucky8,   lucky8,   lucky8,   wingco_state,   init_lucky8a,   ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 4, W-4)",                           0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 1989, lucky8d,   lucky8,   lucky8,   lucky8d,  wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 5, W-4, main 40%, d-up 60%)",       0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 1989, lucky8e,   lucky8,   lucky8,   lucky8d,  wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "New Lucky 8 Lines (set 6, W-4, main 40%, d-up 60%)",       0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 198?, ns8lines,  0,        lucky8,   lucky8b,  wingco_state,   empty_init,     ROT0, "<unknown>",         "New Lucky 8 Lines / New Super 8 Lines (W-4)",              0,                     layout_lucky8p1 )  // only 1 control set...
GAMEL( 1985, ns8linesa, ns8lines, lucky8,   lucky8b,  wingco_state,   empty_init,     ROT0, "Yamate (bootleg)",  "New Lucky 8 Lines / New Super 8 Lines (W-4, Lucky97 HW)",  0,                     layout_lucky8p1 )  // only 1 control set...
GAMEL( 198?, ns8linew,  ns8lines, lucky8,   ns8linew, wingco_state,   empty_init,     ROT0, "<unknown>",         "New Lucky 8 Lines / New Super 8 Lines (F-5, Witch Bonus)", 0,                     layout_lucky8 )    // 2 control sets...
GAMEL( 198?, ns8linewa, ns8lines, lucky8,   ns8linwa, wingco_state,   empty_init,     ROT0, "<unknown>",         "New Lucky 8 Lines / New Super 8 Lines (W-4, Witch Bonus)", 0,                     layout_lucky8p1 )  // only 1 control set...
GAMEL( 198?, super972,  ns8lines, lucky8,   ns8linew, wingco_state,   empty_init,     ROT0, "<unknown>",         "Super 97-2 (Witch Bonus)",                                 MACHINE_NOT_WORKING,   layout_lucky8 )    // ???
GAME(  198?, luckybar,  0,        lucky8,   ns8linew, wingco_state,   empty_init,     ROT0, "<unknown>",         "Lucky Bar (W-4 with mc68705 MCU)",                         MACHINE_NOT_WORKING )  // MC68705 MCU
GAME(  198?, chryangla, ncb3,     lucky8,   ns8linew, wingco_state,   empty_init,     ROT0, "<unknown>",         "Cherry Angel (encrypted, W-4 hardware)",                   MACHINE_NOT_WORKING )
GAMEL( 198?, kkotnoli,  0,        kkotnoli, kkotnoli, goldstar_state, empty_init,     ROT0, "hack",              "Kkot No Li (Kill the Bees)",                               MACHINE_IMPERFECT_COLORS, layout_lucky8 )
GAME(  198?, ladylinr,  0,        ladylinr, ladylinr, goldstar_state, empty_init,     ROT0, "TAB Austria",       "Lady Liner",                                               0 )
GAME(  198?, wcat3,     0,        wcat3,    lucky8,   wingco_state,   empty_init,     ROT0, "E.A.I.",            "Wild Cat 3",                                               MACHINE_NOT_WORKING )

GAME(  1985, luckylad,  0,        lucky8,   luckylad, wingco_state,   init_luckylad,  ROT0, "Wing Co., Ltd.",    "Lucky Lady (Wing, encrypted)",                             MACHINE_NOT_WORKING )  // encrypted (see notes in rom_load)...
GAME(  1991, megaline,  0,        megaline, megaline, unkch_state,    empty_init,     ROT0, "Fun World",         "Mega Lines",                                               MACHINE_NOT_WORKING )

GAMEL( 1993, bingowng,  0,        bingowng, bingowng, wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "Bingo (set 1)",                                            0,                     layout_bingowng )
GAMEL( 1993, bingownga, bingowng, bingownga,bingownga,wingco_state,   empty_init,     ROT0, "Wing Co., Ltd.",    "Bingo (set 2)",                                            0,                     layout_bingowng )

GAME(  2002, mbs2euro,  0,        mbstar,   mbstar,   wingco_state,   empty_init,     ROT0, "Auto-Data Graz",    "Mega Bonus Star II (Euro, Millennium Edition)",            MACHINE_NOT_WORKING )  // need more work in memory map, inputs, and reels alignment.


// --- Flaming 7's hardware (W-4 derivative) ---
GAME(  199?, fl7_3121,  0,        flam7_w4, flam7_w4, wingco_state,   empty_init,     ROT0, "Cyberdyne Systems", "Flaming 7 (W4 Hardware, Red, White & Blue 7's + Hollywood Nights)",          0 )
GAME(  199?, fl7_50,    0,        flaming7, flaming7, wingco_state,   init_flaming7,  ROT0, "Cyberdyne Systems", "Flaming 7 (Custom Hardware, Main, 50 Bonus)",              MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS )
GAME(  199?, fl7_500,   fl7_50,   flaming7, flaming7, wingco_state,   init_flaming7,  ROT0, "Cyberdyne Systems", "Flaming 7 (Custom Hardware, Main, 500 Bonus)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS )
GAME(  199?, fl7_2000,  fl7_50,   flaming7, flaming7, wingco_state,   init_flaming7,  ROT0, "Cyberdyne Systems", "Flaming 7 (Custom Hardware, Main, 2000 Bonus)",            MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS )
GAME(  199?, fl7_2k16,  fl7_50,   flaming7, flaming7, wingco_state,   init_flaming7,  ROT0, "Cyberdyne Systems", "Flaming 7 (Custom Hardware, Egyptian Gold, 2000 Bonus)",   MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS )
GAME(  199?, fl7_tw,    fl7_50,   flam7_tw, flaming7, wingco_state,   init_flam7_tw,  ROT0, "Cyberdyne Systems", "Flaming 7 (Taiwanese Hardware, unknown version)",          MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS )  // needs proper reels gfx roms decryption.


// --- Wing W-6 hardware ---
GAME(  1986, feverch,   0,        feverch,  feverch,  unkch_state,    empty_init,     ROT0, "Wing Co., Ltd.",    "Fever Chance (W-6)",                                       MACHINE_NOT_WORKING )  // inputs, reels, etc..


// --- Wing W-8 hardware ---
GAME(  1990, bonusch,   0,        bonusch,  bonusch,  unkch_state,    empty_init,     ROT0, "Wing Co., Ltd.",    "Bonus Chance (W-8)",                                       MACHINE_NOT_WORKING )  // M80C51F MCU


// --- Magical Odds hardware ---
GAME(  1992, magodds,   0,        magodds,  magodds,  wingco_state,   empty_init,     ROT0, "Pal Company / Micro Manufacturing Inc.", "Magical Odds (set 1)",                             MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
GAME(  1992, magoddsa,  magodds,  magodds,  magodds,  wingco_state,   empty_init,     ROT0, "Pal Company / Micro Manufacturing Inc.", "Magical Odds (set 2)",                             MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
GAME(  1992, magoddsb,  magodds,  magodds,  magodds,  wingco_state,   empty_init,     ROT0, "Pal Company / Micro Manufacturing Inc.", "Magical Odds (set 3)",                             MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
GAME(  1991, magoddsc,  magodds,  magodds,  magoddsc, wingco_state,   init_magoddsc,  ROT0, "Pal Company",                            "Magical Odds (set 4, custom encrypted CPU block)", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING |MACHINE_NO_SOUND)
GAME(  1991, magoddsd,  magodds,  magodds,  magoddsc, wingco_state,   init_magoddsc,  ROT0, "Pal Company",                            "Magical Odds (set 5, custom encrypted CPU block)", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING |MACHINE_NO_SOUND)


/* --- Amcoe games --- */
/*     YEAR  NAME       PARENT    MACHINE   INPUT      STATE           INIT       ROT    COMPANY    FULLNAME                                                       FLAGS              LAYOUT  */
GAMEL( 1997, schery97,  0,        amcoe1,   schery97,  cmaster_state,  init_schery97,  ROT0, "Amcoe",   "Skill Cherry '97 (Talking ver. sc3.52)",                       0,                 layout_nfb96 )  /* running in CB hardware */
GAMEL( 1997, schery97a, schery97, amcoe1,   schery97,  cmaster_state,  init_schery97a, ROT0, "Amcoe",   "Skill Cherry '97 (Talking ver. sc3.52c4)",                     0,                 layout_nfb96 )  /* running in C4 hardware */
GAMEL( 1998, skill98,   0,        amcoe1,   schery97,  cmaster_state,  init_skill98,   ROT0, "Amcoe",   "Skill '98 (Talking ver. s98-1.33)",                            0,                 layout_skill98 )
GAMEL( 1997, pokonl97,  0,        amcoe1,   pokonl97,  cmaster_state,  init_po33,      ROT0, "Amcoe",   "Poker Only '97 (Talking ver. 3.3)",                            0,                 layout_pokonl97 )
GAME(  1998, match98,   0,        amcoe1a,  match98,   cmaster_state,  init_match133,  ROT0, "Amcoe",   "Match '98 (ver. 1.33)",                                        0 )


/* The Sub-PCB has a printed sticker denoting C1, C2, D or DK for the type of FPGA decryption chip used */
/* There is known to be a special IOWA version running on the Texas C2 hardware with roms FB96P IA, FB96L IA & FB96H IA with a (c) 2000 Amcoe */
GAMEL( 1996, nfb96,     0,        amcoe2,   nfb96,     cmaster_state,  init_nfb96_c1, ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (v3.63, C1 PCB)",          0,                 layout_nfb96 ) /* ver. 02-3.63 C1 Sub-PCB */
GAMEL( 1996, nfb96a,    nfb96,    amcoe2,   nfb96,     cmaster_state,  init_nfb96_c1, ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (v3.62, C1 PCB)",          0,                 layout_nfb96 ) /* ver. 00-3.62 C1 Sub-PCB */
GAMEL( 1996, nfb96b,    nfb96,    amcoe2,   nfb96,     cmaster_state,  init_nfb96_d,  ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (v3.54, D PCB)",           0,                 layout_nfb96 ) /* ver. 00-3.54 D Sub-PCB */
GAMEL( 1996, nfb96c,    nfb96,    amcoe2,   nfb96,     cmaster_state,  init_nfb96_dk, ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (v3.62, DK PCB)",          0,                 layout_nfb96 ) /* ver. 00-3.62 DK Sub-PCB */
GAMEL( 2000, nfb96txt,  nfb96,    amcoe2,   nfb96tx,   cmaster_state,  init_nfb96_c2, ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (v1.22 Texas XT, C2 PCB)", 0,                 layout_nfb96 ) /* ver. tf1.22axt C2 Sub-PCB */

GAMEL( 1996, nc96,      0,        amcoe2,   nfb96,     cmaster_state,  init_nfb96_c1, ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.63, C1 PCB)",               0,                 layout_nfb96 ) /* C1 Sub-PCB */
GAMEL( 1996, nc96a,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_c1, ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.62, C1 PCB)",               0,                 layout_nfb96 ) /* C1 Sub-PCB */
GAMEL( 1996, nc96b,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_c1, ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.61, C1 PCB)",               0,                 layout_nfb96 ) /* C1 Sub-PCB */
GAMEL( 1996, nc96c,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_d,  ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.54, D PCB)",                0,                 layout_nfb96 ) /* D  Sub-PCB */
GAMEL( 1996, nc96d,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_d,  ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.53, D PCB)",                0,                 layout_nfb96 ) /* D  Sub-PCB */
GAMEL( 1996, nc96e,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_d,  ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.40, D PCB)",                0,                 layout_nfb96 ) /* D  Sub-PCB */
GAMEL( 1996, nc96f,     nc96,     amcoe2,   nfb96,     cmaster_state,  init_nfb96_dk, ROT0, "Amcoe",   "New Cherry '96 Special Edition (v3.62, DK PCB)",               0,                 layout_nfb96 ) /* DK Sub-PCB */
GAMEL( 2000, nc96txt,   nc96,     amcoe2,   nfb96tx,   cmaster_state,  init_nfb96_c2, ROT0, "Amcoe",   "New Cherry '96 Special Edition (v1.32 Texas XT, C2 PCB)",      0,                 layout_nfb96tx ) /* ver. tc1.32axt C2 Sub-PCB */

GAME(  2009, fb2010,    0,        amcoe2,   fb2010,    cmaster_state,  init_fb2010,   ROT0, "Amcoe",   "Fruit Bonus 2010",                                             0 )

GAMEL( 1996, roypok96,  0,        amcoe2,   roypok96,  cmaster_state,  init_rp35,     ROT0, "Amcoe",   "Royal Poker '96 (set 1, v97-3.5)",                             0,                 layout_roypok96 )
GAMEL( 1996, roypok96a, roypok96, amcoe2,   roypok96a, cmaster_state,  init_rp36,     ROT0, "Amcoe",   "Royal Poker '96 (set 2, v98-3.6)",                             0,                 layout_roypok96 )
GAMEL( 1996, roypok96b, roypok96, amcoe2,   roypok96a, cmaster_state,  init_rp36c3,   ROT0, "Amcoe",   "Royal Poker '96 (set 3, v98-3.6?)",                            0,                 layout_roypok96 )
GAME(  1996, roypok96c, roypok96, amcoe2,   roypok96a, cmaster_state,  init_rp96sub,  ROT0, "Amcoe",   "Royal Poker '96 (set 4, C3 board)",                            MACHINE_NOT_WORKING )


/* these all appear to be graphic hacks of 'New Fruit Bonus '96', they can run with the same program rom
   some sets are messy and appear to have mismatched graphic roms, they need to be sorted out properly
*/
/*    YEAR  NAME       PARENT    MACHINE   INPUT      STATE           INIT            ROT    COMPANY    FULLNAME                                                                   FLAGS  */
GAME( 1996, nfb96se,   nfb96,    amcoe2,   nfb96bl,   cmaster_state,  empty_init,     ROT0, "bootleg", "New Fruit Bonus '96 Special Edition (bootleg set 1, v97-3.3c Portuguese)", 0 )
GAME( 1996, nfb96sea,  nfb96,    amcoe2,   nfb96bl,   cmaster_state,  init_nfb96sea,  ROT0, "bootleg", "New Fruit Bonus '96 Special Edition (bootleg set 2, v97-3.3c English)",    MACHINE_WRONG_COLORS ) // encrypted program
GAME( 1996, nfb96seb,  nfb96,    amcoe2,   nfb96bl,   cmaster_state,  empty_init,     ROT0, "bootleg", "New Fruit Bonus '96 Special Edition (bootleg set 3, v97-3.3c Portuguese)", MACHINE_WRONG_COLORS )
GAME( 1996, nfb96sec,  nfb96,    amcoe2,   nfb96bl,   cmaster_state,  empty_init,     ROT0, "bootleg", "New Fruit Bonus '96 Special Edition (bootleg set 4, v97-3.3c English)",    MACHINE_WRONG_COLORS )
GAME( 2002, carb2002,  nfb96,    amcoe2,   nfb96bl,   cmaster_state,  empty_init,     ROT0, "bootleg", "Carriage Bonus 2002 (bootleg)",                                            MACHINE_WRONG_COLORS )
GAME( 2003, carb2003,  nfb96,    amcoe2,   nfb96bl,   cmaster_state,  empty_init,     ROT0, "bootleg", "Carriage Bonus 2003 (bootleg)",                                            MACHINE_WRONG_COLORS )

GAME( 2003, nfm,       0,        nfm,      nfm,       cmaster_state,  empty_init,     ROT0, "Ming-Yang Electronic", "New Fruit Machine (Ming-Yang Electronic)",                    MACHINE_NOT_WORKING ) // vFB02-07A "Copyright By Ms. Liu Orchis 2003/03/06"


// super cherry master sets...
GAMEL(1994, scmaster, 0,         unkch,    unkch4,    unkch_state,    init_unkch4,    ROT0, "bootleg", "Super Cherry Master (v1.0)",                                   0,    layout_unkch )

// these have 'cherry 1994' in the program roms, but also "Super Cherry / New Cherry Gold '99". probably hacks of a 1994 version of Super Cherry Master.
GAMEL(1999, unkch1,   scmaster,  unkch,    unkch,     unkch_state,    init_unkch1,    ROT0, "bootleg", "New Cherry Gold '99 (bootleg of Super Cherry Master) (set 1)", 0,    layout_unkch )
GAMEL(1999, unkch2,   scmaster,  unkch,    unkch,     unkch_state,    init_unkch1,    ROT0, "bootleg", "Super Cherry Gold (bootleg of Super Cherry Master)",           0,    layout_unkch )
GAMEL(1999, unkch3,   scmaster,  unkch,    unkch3,    unkch_state,    init_unkch3,    ROT0, "bootleg", "New Cherry Gold '99 (bootleg of Super Cherry Master) (set 2)", 0,    layout_unkch ) // cards have been hacked to look like barrels, girl removed?
GAMEL(1999, unkch4,   scmaster,  unkch,    unkch4,    unkch_state,    init_unkch4,    ROT0, "bootleg", "Grand Cherry Master (bootleg of Super Cherry Master)",         0,    layout_unkch ) // by 'Toy System' Hungary

GAME( 1996, cherry96, scmaster,  unkch,    unkch4,    unkch_state,    init_unkch4,    ROT0, "bootleg", "New Cherry '96 (bootleg of New Fruit Bonus?)",                 MACHINE_NOT_WORKING ) // need to be moved to another machine...


/* Stealth sets.
   These have hidden games inside that can be switched to avoid inspections, police or whatever purposes)... */

/*    YEAR  NAME        PARENT    MACHINE   INPUT     STATE           INIT          ROT    COMPANY                FULLNAME                                                      FLAGS                     LAYOUT    */
GAMEL( 198?, cmpacman,  0,        cm,       cmpacman, cmaster_state,  init_cm,      ROT0, "<unknown>",           "Super Pacman (v1.2) + Cherry Master (Corsica, v8.31, set 1)", 0,                        layout_cmpacman ) // need to press K to switch between games...
GAMEL( 198?, cmpacmana, cmpacman, cm,       cmpacman, cmaster_state,  init_cm,      ROT0, "<unknown>",           "Super Pacman (v1.2) + Cherry Master (Corsica, v8.31, set 2)", 0,                        layout_cmpacman ) // need to press K to switch between games...
GAMEL( 198?, cmtetris,  0,        cm,       cmtetris, cmaster_state,  init_cm,      ROT0, "<unknown>",           "Tetris + Cherry Master (Corsica, v8.01, set 1)",              0,                        layout_cmpacman ) // need to press K/L to switch between games...
GAMEL( 198?, cmtetrsa,  0,        cm,       cmtetris, cmaster_state,  init_cm,      ROT0, "<unknown>",           "Tetris + Cherry Master (Corsica, v8.01, set 2)",              MACHINE_NOT_WORKING,      layout_cmpacman ) // seems banked...
GAMEL( 198?, cmtetrsb,  0,        cm,       cmtetris, cmaster_state,  init_cm,      ROT0, "<unknown>",           "Tetris + Cherry Master (+K, Canada Version, encrypted)",      MACHINE_NOT_WORKING,      layout_cmpacman ) // different Tetris game. press insert to throttle and see the attract running.
GAMEL( 1997, crazybon,  0,        pkrmast,  crazybon, goldstar_state, empty_init,   ROT0, "bootleg (Crazy Co.)", "Crazy Bonus 2002 (Ver. 1, set 1)",                            MACHINE_IMPERFECT_COLORS, layout_crazybon ) // Windows ME desktop... but not found the way to switch it.
GAMEL( 1997, crazybona, crazybon, pkrmast,  crazybon, goldstar_state, empty_init,   ROT0, "bootleg (Crazy Co.)", "Crazy Bonus 2002 (Ver. 1, set 2)",                            MACHINE_IMPERFECT_COLORS, layout_crazybon )

/* other possible stealth sets:
 - cmv4a    ---> see the 1fxx zone. put a bp in 1f9f to see the loop.
                 the game has tetris graphics inside.
*/
