// license:BSD-3-Clause
// copyright-holders: Roberto Fresca

/****************************************************************************************

  GAME-A-TRON gambling hardware.
  Driver by Roberto Fresca.

  Games running on this hardware:

  * Four In One Poker,  1983, Game-A-Tron.
  * Pull Tabs,          1983, Game-A-Tron.
  * Bingo,              1983, Game-A-Tron.


*****************************************************************************************

  Hardware Notes:
  ---------------

  * PCB1: PULL TABS.

  Board silkscreend: "GAME-A-TRON CORP. (c)1983 PAT.PENDING"
                     "9003"

  ROMS: - U31 - 2732 (PT-1R-V)
        - U32 - 2732 (PT-2G-V)
        - U33 - 2732 (PT-3B-V)

        - U00 - 2764 (PULL-TABS-1-90)

  Most chips, except for the EPROMS, are covered with a hard black plastic coat
  so their ID's could not be read.

  U30 location was silkscreend VRAM,    24 pin.
  U11 location was silkscreend SCP RAM, 24 pin.
  U13 location was silkscreend CPU,     40 pin.
  U12 location was silkscreend DECODE,  16 pin.

  1x 16MHz Crystal
  1x Duracell PX-2 I, 4.5V battery.


  * PCB2: FOUR IN ONE POKER.

  Board silkscreend: "GAME-A-TRON CORP. (c)1983 PAT.PENDING"
                     "9003"

  ROMS: - U31 - 2732 (POKER-R)
        - U32 - 2732 (POKER-G)
        - U33 - 2732 (BLACK)

        - U00 - 2764 (2764-POKER)
        - U08 - 2732 (2732-POKER-4-1)

  Most chips, except for the EPROMS, are covered with a hard black plastic coat
  so their ID's could not be read.

  U13    CPU (also coated with black plastic).
  U05    I/O        M5L8255AP-5
  U11    SCP RAM    TC5516APL  (2Kx8)
  U30    VRAM       HM6116LP-3 (2Kx8)

  1x 16MHz Crystal
  1x Duracell PX-2 I, 4.5V battery.


  Identified the unknown writes as a init sequence for 1x PSG sound device.
  Is a SN76489/496 family device, and can't be identified accurately due to
  almost all devices being plastic covered.


  * PCB 3: BINGO.

  The PCB doesn't looks like an official Game-A-Tron board. Maybe it's a bootleg,
  or prototype. Battery backed RAM was replaced by a Mostek MK48Z02B-20 zeropower RAM.

   PCB Layout:
  .-----------------------------------------------------------------------------------------------.
  |                                                                                               |
  |    U14        U11        U12        U13        U10        U9                                  |
  |   .--------. .--------. .--------. .--------. .--------. .--------.          REV B            |
  |   |74LS161 | |74LS161 | |74LS161 | |74LS161 | |74LS161 | |HD74LS04|  D R                      |
  |   '--------' '--------' '--------' '--------' '--------' '--------'                           |
  |   .--------. .--------. .--------.                           XTAL                             |
  |   |74LS157 | |74LS157 | |74LS157 |                      R   .-----.  C C                      |
  |   '--------' '--------' '--------'                      R   | 16  |                           |
  |                                                         R   | MHz |                           |
  |                                                             '-----'                           |
  |                                                                               .--------.      |
  |    .-------------.   U26                                .-------------.       | 74LS32 |U28   |
  |    |U23          |  .--------.                          |   Mostek    |U3     '--------'      |
  |    |    2732     |  |74LS166 |                          | MK48Z02B-20 |       .--------.      |
  |    |          ROM|  '--------'                          |zeropower RAM|       | 74LS08 |U31   |
  |    '-------------'                U20                   '-------------'       '--------'      |
  |    .-------------.   U25         .-------------.      .---------------.       .--------.      |
  |    |U22          |  .--------.   |             |      |U2             |       | 74LS00 |U30   |
  |    |    2732     |  |74LS166 |   | NEC D4016C  |      |   2764 type   |       '--------'      |
  |    |          ROM|  '--------'   |   (32Kx8)   |      |            ROM|       .--------.      |
  |    '-------------'               '-------------'      '---------------'       | 74LS10 |U29   |
  |    .-------------.   U24          U18                .---------------------.  '--------'      |
  |    |U21          |  .--------.   .-----------.       |U1                   |  .--------.      |
  |    |    2732     |  |74LS166 |   | 74LS245N  |       |    EMPTY SOCKET     |  | NO IC  |U5    |
  |    |          ROM|  '--------'   '-----------'       |       (Z80)         |  '--------'      |
  |    '-------------'                                   '---------------------'  .--------.      |
  |                      U19            U6               .---------------------.  | 74LS74 |U8    |
  |            .-----.  .--------.     .--------.        |U4                   |  '--------'      |
  |            | POT |  |74LS367 |     |SN76489N|        |    EMPTY SOCKET     |                  |
  |            '-----'  '--------'     '--------'        |     (8255 PPI)      |  .--------.      |
  |             VOLUME               .-----------.       '---------------------'  | 74LS04 |U32   |
  |                              U27 | 74LS374B1 |                                '--------'      |
  |                                  '-----------'    CCCCCCCC      CCCCCCCC                      |
  |                 U7                                                                            |
  |                .--------.          RRRRRRRR       RRRRRRRR      RRRRRRRR       RRRRRRRR       |
  |                | LM380N |                                                                     |
  |                '--------'          CCCCCCCC       RRRRRRRR      RRRRRRRR       CCCCCCCC       |
  |      C    C       C                                                              D  D         |
  |                                                                                               |
  '----------------------------.              25x2 EDGE CONNECTOR                .----------------'
                               | | | | | | | | | | | | | | | | | | | | | | | | | |
  R = Resistors.               | | | | | | | | | | | | | | | | | | | | | | | | | |
  C = Capacitors.              '-------------------------------------------------'
  D = Diodes.


*****************************************************************************************

  *** Game Notes ***


  All games:

  The first time the machine is turned on, it will show the legend "DATA ERROR".
  You must RESET (F3) the machine to initialize the NVRAM properly.

  NOTE: These games are intended to be for amusement only.
  There is not such a payout system, so... Don't ask about it!


  * Four In One Poker:

  Pressing SERVICE 1 (key 9) you enter the Test/Settings Mode. You can test
  inputs there, and change all the game settings. Press "DISCARD 1" (key Z)
  to choose an option, and "DISCARD 5" (key B) to change the settings.
  Press "SERVICE 2" (key 0) to exit.

  The settings options are:

      HIGHEST-ANTE-IS:   1-5-10-15-20-25-30-35-40-45-50.
      JOKERS:            0-1-2.
      BONUS DRAWS:       0-1.
      DOUBLE-UPS:        0-1-2-3-4-5-6-7-8-9.
      WIN-ON:            JACKS AND UP - PAIR OF ACES.
      SKILL LEVEL:       50-55-60-65-70-75-80-85-90-95-100.
      CREDITS-PER-COIN:  1-5-10-15-20-25-30-35-40-45-50-55-60-65-70-75-80-85-90-95-100.

  The game allow to choose one of the following card games:

  - DRAW POKER.
  - STUD POKER.
  - ACEY-DEUCY.
  - BLACKJACK.
  - HIGH-LOW.

  Press "DISCARD 1" (key Z) to switch between games.
  Press "BET/ANTE" (key N) to bet credits and then start the game.

  The rest of buttons are self-explanatory.


  * Pull Tabs:

  Pressing SERVICE 1 (key 9) you enter the Test/Settings Mode. You can test
  inputs there, and change all the game settings. Press "SUPER STAR TICKET"
  (key Z) to choose an option, and "BIG BAR TICKET" (key C) to change the
  settings. Press "SERVICE 2" (key 0) to exit.

  The settings options are:

      HIGHEST-ANTE-IS:   1-5-10-15-20-25.
      SKILL LEVEL:       50-55-60-65-70-75-80-85-90-95-100.
      CREDITS-PER-COIN:  1-5-10-15-20-25-30-35-40-45-50-55-60-65-70-75-80-85-90-95-100.
      MUSIC:             PLAYS - OFF

  You must bet through "ANTE" (key 1), and then choose a ticket to play.

  Press "SUPER STAR TICKET" (key Z) to play with Super Star (left) Ticket.
  Press "LADY LUCK TICKET" (key X) to play with Lady Luck (center) Ticket.
  Press "BIG BAR TICKET" (key C) to play with Big Bar (right) Ticket.

  A curiosity...

  The Pull Tabs flyer shows the following paytable:

  - Super Star -      - Lady Luck -         - Big Bar -
   3x Stars  75     3x Oranges     100     3x Bars     75
   3x Hearts  8     3x Watermelons  25     3x Cherries 10
   3x Cups    4     3x Horseshoes   10     3x Pears     6
   3x Clubs   3     3x Liquors       5     3x Plums     4
   3x Crowns  1     3x Bells         2     3x Bananas   2

  ...but the game seems to have inverted objects importance:

  - Super Star -      - Lady Luck -         - Big Bar -
   3x Crowns 75     3x Bells       100     3x Bananas  75
   3x Clubs   8     3x Liquors      25     3x Plums    10
   3x Cups    4     3x Horseshoes   10     3x Pears     6
   3x Hearts  3     3x Watermelons   5     3x Cherries  4
   3x Stars   1     3x Oranges       2     3x Bars      2

  Can't get an input or combination of them that change this logic.
  Maybe the paytable is different in this set, or just the flyer doesn't
  reflect the real thing.


  * Bingo:

  Pressing SERVICE 1 (key 9) you enter the Test/Settings Mode. You can test
  inputs there, and change all the game settings. Press "CHANGE CARD" (key Z)
  to choose an option, and "CHANGE GAME" (key C) to change the settings.
  Press "SERVICE 3" (key 8) to test audio. Press "SERVICE 2" (key 0) to exit.

  The settings options are:

      HIGHEST-ANTE-IS:             1-5-10-15-20-25-30-35-40-45-50.
      X OR FEWER HITS WIN 1 to 1:  1-2-3-4-5-6.
      X DOUBLE-UPS:                0-1-2-3-4-5-6-7-8-9.
      BEEPS DURING PLAY:           YES-NO.
      SKILL LEVEL:                 50-55-60-65-70-75-80-85-90-95-100.
      CREDITS-PER-COIN:            1-5-10-15-20-25-30-35-40-45-50-55-60-65-70-75-80-85-90-95-100.
      3 ON A LINE WINS:            YES-NO.

  You must bet through "ANTE" (key 1), and then...

  Press "CHANGE CARD" (key Z) to change for another card with a different set of numbers.
  Press "START" (key X) to start the game.
  Press "CHANGE GAME" (key C) to switch between games X-L-T-C-N-U.

  Note that letters in games X-L-T-C-N-U are just references to the shape of the special
  numbers group inside the card, which will play.

  You must setup double-ups to something different of 0 (default), to play with these
  features (High or Low ball)


*****************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x0000 - 0x5FFF    ; ROM space.
  0x6000 - 0x67FF    ; Video RAM (only the first 0x300 bytes are used).
  0x8000 - 0x87FF    ; Main RAM.
  0xA000 - 0xA000    ; Sound (PSG).
  0xE000 - 0xE000    ; Output Port 0 (lamps).

  * Z80 I/O ports *

    0x00 - 0x03      ; PPI 8255 (ports A & B as input, port C as output).

  * 8255 I/O ports *

    Port A (input)   ; Input Port 0 (player buttons).
    Port B (input)   ; Input Port 1 (player & service buttons).
    Port C (output)  ; Output Port 1 (lamps & counters).


*****************************************************************************************

  DRIVER UPDATES:

  [2015-10-22]
  - Added siren/alarm input to Pull Tabs, and beeps/alarm input
     to Four In One Poker. All these are present in the Test Mode.
     However, their functions aren't clear.
  - Switched the PSG to SN76489, since it's present in the Bingo PCB.
  - Added technical notes and more documentation.

  [2014-02-04]
  - Added Bingo (1983). PCB seems bootleg, but the game looks legit.
  - Worked from the scratch a whole set of inputs and button-lamps support for this game.
  - Changed the poker41 description to Four In One Poker (as seen in the official brochure).
  - Added game and technical notes.

  [2008-10-14]
  - Improved the button-lamps layouts to look more realistic.

  [2008-08-21]
  After an exhaustive analysis to the unknown writes, finally figured out the missing sound device.
  - Added sound support to "Poker 4-1" and "Pull Tabs".
  - Figured out the output ports. Documented each bit accessed.
  - Added button lamps support. Created layouts for both games.
  - Switched the 8255 port C to be used as output port.
  - Adjusted the coin pulse timing.
  - Updated technical notes.
  - Split the driver to driver + video.
  - Final clean-up.

  [2008-05-31]
  - Renamed the games to "Poker 4-1" and "Pull Tabs" as shown in the ROMs stickers.
  - Renamed the ROMs in each set according to their own stickers.
  - Moved the driver into gametron.a group.
  - Added the missing input port C to 8255 PPI I/O chip. Poker41 and pulltabs don't
     make use of it, but is present in the Test/Settings Mode.
  - Updated technical notes.

  [2008-05-10]
  - Initial release.
  - Properly decoded graphics.
  - Proper memory map.
  - Added NVRAM support.
  - Proper Inputs through 8255 PPI I/O chip.
  - Both games are working.
  - Added technical & game notes.


  TODO:

  - Nothing... :)

*****************************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "bingo.lh"
#include "poker41.lh"
#include "pulltabs.lh"


namespace {

class gatron_state : public driver_device
{
public:
	gatron_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void gat(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<9> m_lamps;
	tilemap_t *m_bg_tilemap = nullptr;

	void output_port_0_w(uint8_t data);
	void output_port_1_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
};


void gatron_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gatron_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    xxxx xxxx   tiles code.

    only one color code
*/

	int const code = m_videoram[tile_index];

	tileinfo.set(0, code, 0, 0);
}

void gatron_state::video_start()
{
	m_lamps.resolve();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gatron_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 16, 48, 16);
}

uint32_t gatron_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/****************************
*    Read/Write Handlers    *
****************************/

void gatron_state::output_port_0_w(uint8_t data)
{
/*---------------
  Poker 4-1 lamps
  ---------------

  0x00 - Default State.
  0x01 - Hold3.
  0x02 - Hold4.
  0x04 - Hold5/DDown.
  0x08 - Ante/Bet.
  0x10 - Start.
  0x20 - Deal/Hit.
  0x40 - Stand/FreeBonusDraw.

  - bits -
  7654 3210
  ---------
  .... ...x --> Hold3.
  .... ..x. --> Hold4.
  .... .x.. --> Hold5/DDown.
  .... x... --> Ante/Bet.
  ...x .... --> Start.
  ..x. .... --> Deal/Hit.
  .x.. .... --> Stand/FreeBonusDraw.


  ---------------
  Pull Tabs lamps
  ---------------

  0x00 - Default State.
  0x01 - Hold3.
  0x04 - Hold5.
  0x08 - Ante/Bet.

  - bits -
  7654 3210
  ---------
  .... ...x ---> Hold3.
  .... .x.. ---> Hold5.
  .... x... ---> Ante/Bet.

  Tab1 = Hold1
  Tab2 = Hold3
  Tab3 = Hold5


  ---------------
  Bingo lamps
  ---------------

  0x01 - unknown.
  0x02 - unknown.
  0x04 - unknown.
  0x08 - Ante/Bet.
  0x10 - Start.
  0x20 - Change Game / D-UP / High.
  0x40 - Change Card / Take / Low.

  - bits -
  7654 3210
  ---------
  .... ...x --> Hold3.
  .... ..x. --> Hold4.
  .... .x.. --> Hold5/DDown.
  .... x... --> Ante/Bet.
  ...x .... --> Start.
  ..x. .... --> Change Game / D-UP / High.
  .x.. .... --> Change Card / Take / Low.

*/
	for (uint8_t i = 0; i < 7; i++)
		m_lamps[i] = BIT(data, i);
}


void gatron_state::output_port_1_w(uint8_t data)
{
/*----------------
  Lamps & Counters
  ----------------

  - bits -
  7654 3210
  ---------
  .... ...x --> Hold2 lamp.
  .... ..x. --> Hold1 lamp.
  .x.. .... --> Coin counter (inverted).
  x... .... --> Inverted pulse. Related to counters.

*/
	for (uint8_t i = 0; i < 2; i++)
		m_lamps[i + 7] = BIT(data, i);
}

/*************************
* Memory Map Information *
*************************/

void gatron_state::prg_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x63ff).ram().w(FUNC(gatron_state::videoram_w)).share(m_videoram);
	map(0x8000, 0x87ff).ram().share("nvram");                          // battery backed RAM
	map(0xa000, 0xa000).w("snsnd", FUNC(sn76489_device::write));       // PSG
	map(0xe000, 0xe000).w(FUNC(gatron_state::output_port_0_w));  // lamps
}

void gatron_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( poker41 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Bet / Ante")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Hit")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Free Bonus Draw / Stand")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5 / High / Double Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )     PORT_NAME("Service 3 (Trigger bips/alarm in Test Mode)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )  PORT_NAME("Service 2 (Test Mode Out / Coin Stuck)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* Payout? */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Service 1 (Test/Settings)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1 / Low")
INPUT_PORTS_END

static INPUT_PORTS_START( pulltabs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_NAME("Ante") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_IMPULSE(2) /* Coin A */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_NAME("Big Bar Ticket") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_NAME("Lady Luck Ticket") PORT_CODE(KEYCODE_X)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service 3 (Trigger siren/alarm in Test Mode)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Service 2 (Test Mode Out / Coin Stuck)") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service 1 (Test/Settings)") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_NAME("Super Star Ticket") PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

static INPUT_PORTS_START( bingo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Ante") PORT_CODE(KEYCODE_1)                // bet/ante
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Change Game / High") PORT_CODE(KEYCODE_C)  // change game (lucky game X-L-T-C-N-U) / change values in settings.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)                                       // coin in
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Change Card / Low")  PORT_CODE(KEYCODE_Z)  // change card / move down in settings
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Start") PORT_CODE(KEYCODE_X)               // start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service 3 (Trigger beeps/alarm in Test Mode)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Service 2 (Test Mode Out / Coin Stuck)") PORT_CODE(KEYCODE_0) // exit test-settings mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service 1 (Test/Settings Mode)") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 16,
	RGN_FRAC(1,3),  // 256 tiles
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    // bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    // every char takes 16 consecutive bytes
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_gat )
	GFXDECODE_ENTRY( "chars", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void gatron_state::gat(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);
	static constexpr XTAL CPU_CLOCK = MASTER_CLOCK / 24;    // 666.66 kHz, guess...

	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gatron_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &gatron_state::port_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.out_pc_callback().set(FUNC(gatron_state::output_port_1_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(48*8, 16*16);
	screen.set_visarea(0*8, 48*8-1, 0*8, 16*16-1);
	screen.set_screen_update(FUNC(gatron_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_gat);
	PALETTE(config, "palette").set_entries(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489(config, "snsnd", MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 2.00);   // Present in Bingo PCB. Clock needs to be verified.
}


/*************************
*        Rom Load        *
*************************/

ROM_START( poker41 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker.u00",      0x0000, 0x2000, CRC(8361fccd) SHA1(4faae6bb3104c1f4a0939d613966085d7e34c1df))
	ROM_LOAD( "poker-4-1.u08",  0x2000, 0x1000, CRC(61e71f31) SHA1(b8d162a47752cff7412b3920ec9dd7a469e81e62) )

	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "black.u33",      0x0000, 0x1000, CRC(3f8a2d59) SHA1(d61dce33aa8637105905830e2f37c1052c441194) )
	ROM_LOAD( "poker-g.u32",    0x1000, 0x1000, CRC(3e7772b2) SHA1(c7499ff148e5a9cbf0958820c41ea09a843ab355) )
	ROM_LOAD( "poker-r.u31",    0x2000, 0x1000, CRC(18d090ec) SHA1(3504f18b3984d16545dbe61a03fbf6b8e2027150) )
ROM_END

ROM_START( pulltabs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pull-tabs-1-90.u00", 0x0000, 0x2000, CRC(7cfd490d) SHA1(8eb360f8f4806a4281dae12236d30aa86d00993d) )

	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "pt-3b-v.u33",    0x0000, 0x1000, CRC(3505cec1) SHA1(98ab0383c4be382aea81ab93433f2f29a075f65d) )
	ROM_LOAD( "pt-2g-v.u32",    0x1000, 0x1000, CRC(4a3f4f36) SHA1(3dc29f78b7df1a433d0b39bfeaa227615e70ceed) )
	ROM_LOAD( "pt-1r-v.u31",    0x2000, 0x1000, CRC(6d1b80f4) SHA1(f2da4b4ae1eb05f9ea02e7495ee8110698cc5d1b) )
ROM_END

ROM_START( bingo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "revb.u2", 0x0000, 0x2000, CRC(0322e2b5) SHA1(e191ad00de56e448a41350e32fb6a4828050a2d4) )

	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "revb.u23",    0x0000, 0x1000, CRC(8d15fc35) SHA1(e66abaead70e6c024efbf177f1a4616449f2d231) )
	ROM_LOAD( "revb.u22",    0x1000, 0x1000, CRC(60254c3b) SHA1(4b9e57a8ac9e6e2c6349d6847bbf3f46232721ad) )
	ROM_LOAD( "revb.u21",    0x2000, 0x1000, CRC(b8cc348b) SHA1(34a4690f6464db17ee363bba8709d0ad63aa7cf1) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME      PARENT  MACHINE  INPUT     CLASS         INIT        ROT   COMPANY         FULLNAME              FLAGS                  LAYOUT
GAMEL( 1983, poker41,  0,      gat,     poker41,  gatron_state, empty_init, ROT0, "Game-A-Tron",  "Four In One Poker",  MACHINE_SUPPORTS_SAVE, layout_poker41  )
GAMEL( 1983, pulltabs, 0,      gat,     pulltabs, gatron_state, empty_init, ROT0, "Game-A-Tron",  "Pull Tabs",          MACHINE_SUPPORTS_SAVE, layout_pulltabs )
GAMEL( 1983, bingo,    0,      gat,     bingo,    gatron_state, empty_init, ROT0, "Game-A-Tron",  "Bingo",              MACHINE_SUPPORTS_SAVE, layout_bingo  )
