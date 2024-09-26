// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/************************************************************************************************

  AVT - ADVANCED VIDEO TECHNOLOGY
  CPU Boards 1000-1 & 1001-1

  Driver by Roberto Fresca.

  TODO:
  - needs CTC and daisy chain to make progresses

  Games running on this hardware:

  * Symbols (ver 1.4),  1985,  AVT.
  * Symbols (ver 2.5),  1985,  AVT.
  * Arrow Bingo,        1985,  AVT.
  * NFL (ver 109),      1989,  AVT.


  Special thanks to Dave Ormiston for his invaluable collaboration,
  providing dumps and other technical stuff.


*************************************************************************************************


  Hardware Notes:
  ---------------

  CPU: 1x Z80
  PIO: 2x Mostek MK3881 (U22 & U23)
  CTC: 1x Mostek MK3882 (U27)
  SND: 1x AY-3-8910 or similar.

  CRTC: 1x MC6845 type...

  Unknown Xtal.


*************************************************************************************************

  ------------------
  *** Game Notes ***
  ------------------


  ********** ARROW BINGO **********

  Arrow Bingo is a skill game with the ever-popular Bingo theme. Columns are moved up and down
  to line up the winning numbers with the time clock running... (from the flyer).

  The code has a timer that takes a couple of minutes to boot into the game.
  This is completely normal and expected.



  ************ SYMBOLS ************

  INTRODUCTION

  Symbols is a video skill game displaying a large red bordered box with a white grid of 5 rows
  across and 5 columns of symbols down. The object of the game is to line up the winning symbols
  either horizontally or diagonally in the shortest amount of time with the least amount of moves.

  The winning symbols are selected at random at the start of the game and displayed with a
  distinctive yellow color surrounding that symbol in each column.


  HOW TO PLAY THE GAME:

  The player must first insert coins to obtain points to play the game. Once points are on the
  screen the player may then press the play button one time for each point available up to the
  coin multiplier limit (except when lockouts are in effect) at which time the game will auto
  start. If the player wants to start before the coin max, he may press the start button to
  initiate play. At the start of the game arrows are displayed at the top of columns to show the
  direction to move that column in order to get the winning symbol on screen, in the event that
  particular winning symbol is already on screen, that symbol will appear at the top of the
  column instead of the arrow. When the player has all the winning symbols on the screen an
  alignment of winning symbols, either horizontally or diagonally, should be done with the least
  amount of moves in the shortest amount of time. In the event the lockout mode is engaged the
  game will accept coins up to the lockout limit and will then auto start.

  The game has two different win modes defined as follows. If an alignment is obtained in 13
  moves or less, the player will be awarded points defined by the award tables. If alignment
  is obtained from 14 to 32, moves stars will then be awarded for the amount of moves as
  defined by the awards table. A bonus of 3 stars will be awarded if game is completed in 3
  seconds or less and 100 stars equals 1 point.

  Time is also an element in this game. When the game is started the player has a specific
  amount of time to complete the game, also the arrows have a time limit. These time factors
  are variable and may be adjusted in the setup mode. The game also show how the game may be
  played.


  ADJUSTABLE FEATURES:

  Symbols has been designed for the convenience of the operator. The game is very flexible,
  easy to use and can be custom tailored for each location by a menu driven setup mode.
  To enter the setup mode, locate 2 small switches, with black rubber tops, on the logic board
  labeled SW1 and SW2. First depress and hold SW2 and momentarily depress SW1, until the MAIN
  MENU display appears on the screen. At this point you should see a menu that looks as follows:


       MAIN MENU
      -----------
    ACCOUNTING MODE
    POINTS SETUP
    PLAY TIME SETUP
    BUSINESS NAME SETUP
    SYSTEM TESTS
    GAME STATS


  When you have this display on the screen you are in the setup mode. You will note a different
  color band around one of the selections. This band indicates which module is to be selected
  to enter that particular mode. This band may be moved by depressing either the first column
  up or down arrow on your front panel. To enter a particular mode just depress the PLAY button
  on your front panel. To exit a mode just depress the START button on your front panel. To
  adjust an entry in one of the modules use the second up or down arrow on your front panel. Now
  lets go through each module so you know how to customize your game for a particular location.

  Let's get into the first module on the menu which is the ACCOUNTING MODE. Remember to depress
  the PLAY button on your front panel and presto, we are in the accounting mode. The display you
  see should look something like this:


    ACCOUNTING MODE
    ----------------
    COINS IN:          0000
    POINTS AWARDED:    0000
    PLAYS TO DATE:     0000
    STRUNG COINS:      0000
    STARS AWARDED:     0000
    GAMES PLAYED:      0000


  To clear the accounting back to zero, use the second arrow down button on your front panel for
  each item and use your first arrow up or down to move to each item.


  To enter POINTS SETUP, move the shaded color bar to POINTS SETUP and depress the PLAY button on
  your front panel. You should have a display that looks as follows:


    POINTS MENU
    --------------
    POINTS SETUP
    POINTS MULTIPLIER


  Select the POINTS SETUP by depressing the PLAY button on your front panel and you should have
  a display that looks similar to the following:


    POINTS SETUP
    --------------
    POINTS MENU
    1        100
    2        50
    3        20
    4        10
    5        9
    6        8
    7        7
    8        6
    9        5
    10       4
    11       3
    12       2
    13       1


  To change any of the above values, move the shaded bar by depressing the first up or down arrow
  to the desired award and then change the values by depressing the second up or down arrow until
  the desired value is reached. To exit, press the START button on your front panel.

  To change the POINT MULTIPLIER, enter this module and use the first up or down button on your
  front panel to increment or decrement this value.

  To change PLAY TIME SETUP, enter this module and you may change PLAY TIME or ARROW TIME (the
  amount of time arrows are allowed to stay on screen during play). To enable the lockout mode, you
  may set the lockout number to 1 or 2 or 3 or 4 and 0 if no lockout is desired. To disable the
  STARS set the STAR MODE to OFF. To adjust the UP AWARDS set to ON for more upper awards and to
  OFF for less upper awards table wins.

  The default name used in BUSINESS NAME SETUP is "THE MANAGEMENT". If you want to put a location
  name in the attract mode, enter the business name setup and use the second up and down arrows to
  select from the alphabet. In the event you make a mistake you can backup by selecting the first
  up arrow. When you are finished with your message just exit by pressing START button and the
  message will automatically center when you exit.


  SYSTEM TESTS

  The SYSTEM TESTS mode allows you to test the ROM, RAM, CTC, MONITOR, and perform I/O tests.
  The I/O tests allow you to check each front panel switch and the coins in switch individually.
  To exit from the I/O mode, you have to press the SW1 reset switch on the logic board.


  GAME STATS

  The GAME STATS mode indicates the number of wins for each award in the awards tables. To clear
  these back to zero depress the second arrow down button on your front panel.


*************************************************************************************************

  Symbols front panel layout:

            .------. .------. .------. .------. .------.
            |  UP  | |  UP  | |  UP  | |  UP  | |  UP  |
  .-------. |   1  | |   2  | |   3  | |   4  | |   5  | .------.
  | START | '------' '------' '------' '------' '------' | PLAY |
  |       | .------. .------. .------. .------. .------. |      |
  '-------' | DOWN | | DOWN | | DOWN | | DOWN | | DOWN | '------'
            |   1  | |   2  | |   3  | |   4  | |   5  |
            '------' '------' '------' '------' '------'

*************************************************************************************************

  Symbols connector list CPU 1000-1
  ---------------------------------

  *** J3 - Power Connector ***

  Pin #    Description
  --------------------
  01       +12V DC
  02       +5V DC
  03       N/C
  04       GND


  *** J1 - Switches and Lamps Circuits ***

  Pin #    Description           Pin #    Description
  --------------------           --------------------
  01, 02   Lamp #0               31       Down Arrow #1
  03, 04   Lamp #1               32       Down Arrow #2
  05, 06   Lamp #2               33       Down Arrow #3
  07, 08   Lamp #3               34       Down Arrow #4
  09, 10   Lamp #4               35       Down Arrow #5
  11, 12   Lamp #5               36       N/C
  13, 14   Lamp #6               37       Down Arrow commons
  15, 16   Lamp #7               38       Tilt
  17       Up Arrow #1           39       Play
  18       Up Arrow #2           40       Start
  19       Up Arrow #3           41       N/C
  20       Up Arrow #4           42       N/C
  21       Up Arrow #5           43       N/C
  22       N/C                   44       Tilt, Play, Start common
  23       Up Arrow commons      45       N/C
  24       N/C                   46       N/C
  25       N/C                   47       N/C
  26       N/C                   48       N/C
  27       N/C                   49       N/C
  28       N/C                   50       N/C
  29       Coins In
  30       Coins In common


  *** J2 - Monitor & Misc. ***

  Pin #    Description           Pin #            Description
  --------------------           -------------------------------
  33       Horz Sync             01, 02, 03, 04   Coins In Meter
  35       Vert Sync             05, 06           AC Hot
  37       Composite             07, 08           +Lamp
  39       Red                   09, 10           +5V
  41       Green                 11, 12           +5V
  43       Blue                  13, 14           +12V
  45       Gnd                   15, 16           +12V
  47       Speaker +             25, 26           Lockout
  49       Speaker -

-----------------------------------------------------------------------------------

  Interface Board 1001-1 connections
  ----------------------------------

  J3 Pin #    Description            J4 Pin #      Description
  -----------------------            -------------------------
  A, 01       +5V                    A, 01         N/C
  B, 02       +12V                   B, 02, 03     N/C
  C, 03       GND                    C             Coin Meter
  D, E, F     N/C                    D             AC Hot
  04, 05, 06  N/C                    04            Lockout
  H, 07       Lamp +                 E             AC Hot
  J, K, L     N/C                    05 through 21    N/C
  08, 09      N/C                    F through Y      N/C
  10          Lamp #0                Z, 22         Audio -
  M           Not Used               a, 23         Audio +
  11          Lamp #1                b, 24         N/C
  N           Arrow Down #5          c             N/C
  12          Lamp #2                25            Comp Sync
  P           Arrow Down #4          d             Vert Sync
  13          Lamp #3                26            Blue
  R           Arrow Down #3          e             GND
  14          Lamp #4                27            Green
  S           Arrow Down #2          f             Horz Sync
  15          Lamp #5                28            Red
  T           Arrow Down #1
  16          Lamp #6
  U           Coins In common
  17          Lamp #7
  V           Coins In
  18          N/C
  W           Not Used
  19          Arrows Down common
  X           Not Used
  20          Tilt
  21          Play
  22          Start
  a           Not Used
  23          Not Used
  b           Arrows Up common
  24, c       Not Used
  25          Tilt, Play, Start common
  d           Arrow Up #5
  26          N/C
  e           Arrow Up #4
  27          Arrow Up #1
  f           Arrow Up #3
  28          Arrow Up #2


*************************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0000-6000    ; ROM space.
  A000-A7FF    ; Video RAM.
  C000-C7FF    ; Color RAM?.

  ---I/O---

    21-23      ; AY-3-8910
    28-29      ; CRTC (MC6845?)


*************************************************************************************************

  Color System
  ------------

  * Arrow Bingo & Symbols

  Bipolar PROM scheme:

  .-----------.----.----.----.----.----.----.----.----.----.----.----.----.----.----.----.----.
  |Color Code | 00 | 10 | 20 | 30 | 40 | 50 | 60 | 70 | 80 | 90 | A0 | B0 | C0 | D0 | E0 | F0 |
  >-----------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----<
  |   0000:   | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 0C | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0010:   | 02 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 02 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0020:   | 04 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 04 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0030:   | 06 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 06 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0040:   | 08 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 08 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0050:   | 0A | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 0A | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0060:   | 0C | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 0C | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
  |   0070:   | 0E | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 0E | 00 | 00 | 00 | 00 | 00 | 00 | 0F |
  '-----------'----'----'----'----'----'----'----'----'----'----'----'----'----'----'----'----'

  Being....


  7654 3210
  ---- ---x  Unknown (intensity?).
  ---- --x-  Red.
  ---- -x--  Green.
  ---- x---  Blue.
  xxxx ----  Not used.



*************************************************************************************************


  DRIVER UPDATES:


  [2010-10-10]

  - Initial release.
  - Almost accurate memory map.
  - Decoded graphics.
  - Decoded the bipolar PROM.
  - Detected a CRTC (MC6845 type) reversing the code.
  - Guessed the CRTC clock to get proper values.
  - Hooked the AY-3-8910.
  - Added games notes.
  - Added technical notes.


  TODO:

  - Improve the memory map.
  - Reverse the IO port R/W.
  - PIO / CTC / daisy-chain interrupts.
  - Inputs


* avtbingo, avtsym14 will accept coins. PIO-1 is programmed to cause an interrupt
  when coin1 is pressed (port02).

* avtnfl, you need to set it up before it can accept coins. However, it is random if it works
  after doing that. If you want to try, press 7 to enter setup, press 6 to select the adjustments
  screen, choose items with 2 and adjust them with 4. Press 6 until you exit. If you're lucky enough
  to get it to work, 2 will enter coins, 3 will tilt (don't do that!), 6 is Draft, 1,2,4 (at least)
  will remove cards. Don't know what the aim of the game is though.

* avtsym25 has mismatched program roms so cannot work.


************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(16'000'000)          /* unknown */
#define CPU_CLOCK       MASTER_CLOCK/4      /* guess... seems accurate */
#define CRTC_CLOCK      MASTER_CLOCK/24     /* it gives 63.371293 Hz. with current settings */


class avt_state : public driver_device
{
public:
	avt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_pio0(*this, "pio0")
		, m_pio1(*this, "pio1")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void avtnfl(machine_config &config);
	void avt(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void avt_6845_address_w(uint8_t data);
	void avt_6845_data_w(uint8_t data);
	uint8_t avt_6845_data_r();
	void avt_videoram_w(offs_t offset, uint8_t data);
	void avt_colorram_w(offs_t offset, uint8_t data);
	void avtnfl_w(int state);
	void avtbingo_w(int state);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void avt_palette(palette_device &palette) const;
	uint32_t screen_update_avt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void avt_map(address_map &map) ATTR_COLD;
	void avt_portmap(address_map &map) ATTR_COLD;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_crtc_vreg[0x100]{}, m_crtc_index = 0;
	required_device<z80_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


/*********************************************
*               Video Hardware               *
*********************************************/


void avt_state::avt_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void avt_state::avt_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(avt_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    xxxx ----   color code.
    ---- xxxx   seems unused.
*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] | ((attr & 1) << 8);
	int color = (attr & 0xf0)>>4;

	tileinfo.set(0, code, color, 0);
}


void avt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(avt_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 28, 32);

	m_crtc_index = 0;
	std::fill(std::begin(m_crtc_vreg), std::end(m_crtc_vreg), 0);

	save_item(NAME(m_crtc_index));
	save_item(NAME(m_crtc_vreg));
}


uint32_t avt_state::screen_update_avt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	count = 0;

	for(y = 0; y < mc6845_v_display; y++)
	{
		for(x = 0; x < mc6845_h_display; x++)
		{
			uint16_t tile = m_videoram[count] | ((m_colorram[count] & 1) << 8);
			uint8_t color = (m_colorram[count] & 0xf0) >> 4;

			gfx->opaque(bitmap, cliprect, tile,color, 0, 0, x * 8, y * 8);

			count++;
		}
	}
	//m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void avt_state::avt_palette(palette_device &palette) const
{
	/*  prom bits
	    7654 3210
	    ---- ---x   Intensity?.
	    ---- --x-   Red component.
	    ---- -x--   Green component.
	    ---- x---   Blue component.
	    xxxx ----   Unused.
	*/

	/* 0000BGRI */
	const uint8_t *color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int j = 0; j < palette.entries(); j++)
	{
		constexpr int intenmin = 0xe0;
		constexpr int intenmax = 0xff;

		int const i = ((j & 0x7) << 4) | ((j & 0x78) >> 3);

		// intensity component
//      int const inten = BIT(~color_prom[i], 0);
		int const inten = BIT(color_prom[i], 0);

		// red component
		int const r = BIT(color_prom[i], 1) * (inten ? intenmax : intenmin);

		// green component
		int const g = BIT(color_prom[i], 2) * (inten ? intenmax : intenmin);

		// blue component
		int const b = BIT(color_prom[i], 3) * (inten ? intenmax : intenmin);

		// hack to switch cyan->magenta for highlighted background
		if (j == 0x40)
			palette.set_pen_color(j, rgb_t(g, r, b)); // Why this one has R-G swapped?...
		else
			palette.set_pen_color(j, rgb_t(r, g, b));
	}
}


/**********************************************
*            Read / Write Handlers            *
**********************************************/

//void avt_state::debug_w(uint8_t data)
//{
//  popmessage("written : %02X", data);
//}

// [:crtc] M6845: Mode Control 10 is not supported!!!

void avt_state::avt_6845_address_w(uint8_t data)
{
	m_crtc_index = data;
	m_crtc->address_w(data);
}

void avt_state::avt_6845_data_w(uint8_t data)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

uint8_t avt_state::avt_6845_data_r()
{
	//m_crtc_vreg[m_crtc_index] = data;
	return m_crtc->register_r();
}

/*********************************************
*           Memory Map Information           *
*********************************************/

/* avtnfl, avtbingo */
void avt_state::avt_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0x9fff).ram(); // .share("nvram");
	map(0xa000, 0xa7ff).ram().w(FUNC(avt_state::avt_videoram_w)).share("videoram");
	map(0xc000, 0xc7ff).ram().w(FUNC(avt_state::avt_colorram_w)).share("colorram");
}

void avt_state::avt_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pio0, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x08, 0x0b).rw(m_pio1, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0c, 0x0f).rw("ctc0", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x21, 0x21).w("aysnd", FUNC(ay8910_device::data_w));     /* AY8910 data */
	map(0x23, 0x23).w("aysnd", FUNC(ay8910_device::address_w));      /* AY8910 control */
	map(0x28, 0x28).w(FUNC(avt_state::avt_6845_address_w));
	map(0x29, 0x29).rw(FUNC(avt_state::avt_6845_data_r), FUNC(avt_state::avt_6845_data_w));
}

/* I/O byte R/W
  (from avtbingo)

  inputs are through port 02h, masked with 0x3F & 0x40.

  02C3: DB 02         in   a,($02)
  02C5: 2F            cpl
  02C6: E6 3F         and  $3F
  02C8: C9            ret

  02D1: DB 02         in   a,($02)
  02D3: 2F            cpl
  02D4: E6 3F         and  $3F
  02D6: CD B2 02      call $02B2
  02D9: C9            ret

  0338: DB 02         in   a,($02) --> poll IN0
  033A: E6 40         and  $40 ------> check for IN0-7 if active.
  033C: 28 02         jr   z,$0340 --> to continue the program.
  033E: AF            xor  a
  033F: C9            ret
  ....
  1ACB: B7            or   a
  1ACC: 28 03         jr   z,$1AD1 --> to continue the program.
  1ACE: CD B6 2D      call $2DB6 ----> nothing there!!!


  This changes which lamps are lit depending on which ones are lit now..
  poll the port 00h and compare with 0x03

  1379: 0E 00         ld   c,$00
  137B: ED 78         in   a,(c)
  137D: FE 03         cp   $03
  137F: 20 04         jr   nz,$1385
  ...code continues...


  -----------------

  unknown writes:

  [:maincpu] ':maincpu' (01D4): unmapped io memory write to 0001 = 0F & FF
  [:maincpu] ':maincpu' (01D8): unmapped io memory write to 0009 = 4F & FF
  [:maincpu] ':maincpu' (01DC): unmapped io memory write to 000B = CF & FF
  [:maincpu] ':maincpu' (01E0): unmapped io memory write to 000B = C0 & FF

  [:maincpu] ':maincpu' (01E4): unmapped io memory write to 000A = C0 & FF \
  [:maincpu] ':maincpu' (02CD): unmapped io memory write to 000A = EE & FF  \
  [:maincpu] ':maincpu' (030E): unmapped io memory write to 000A = C0 & FF   > Alternate these values too often... Mux selector?
  [:maincpu] ':maincpu' (02CD): unmapped io memory write to 000A = EE & FF  /
  [:maincpu] ':maincpu' (030E): unmapped io memory write to 000A = C0 & FF /

  [:maincpu] ':maincpu' (0321): unmapped io memory write to 0003 = 06 & FF \
  [:maincpu] ':maincpu' (0325): unmapped io memory write to 0003 = CF & FF  \
  [:maincpu] ':maincpu' (0329): unmapped io memory write to 0003 = FF & FF   > Unknown commands.
  [:maincpu] ':maincpu' (032D): unmapped io memory write to 0003 = 97 & FF  /
  [:maincpu] ':maincpu' (0331): unmapped io memory write to 0003 = F7 & FF /

  [:maincpu] ':maincpu' (0335): unmapped io memory write to 000A = C0 & FF \
  [:maincpu] ':maincpu' (02CD): unmapped io memory write to 000A = EE & FF  \
  [:maincpu] ':maincpu' (030E): unmapped io memory write to 000A = C0 & FF   > Same as above...
  [:maincpu] ':maincpu' (02CD): unmapped io memory write to 000A = EE & FF  /
  [:maincpu] ':maincpu' (030E): unmapped io memory write to 000A = C0 & FF /


  avtnfl and avtbingo have similarities.
  avtsym seems different.

  all access a000/c000 with an offset of 0x800 for video.
  avtnfl and avtbingo use 28/29 for CRTC.

  */

/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( symbols )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2") // avtnfl: down/accept
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3") // avtnfl: up
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-5") // avtnfl: adjust setting
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6") // avtnfl: go back a screen
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7") // avtnfl: enter setup menu   avtsym14:hold down while booting to enter setup
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


static INPUT_PORTS_START( avtbingo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Column 3 UP")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Column 2 UP")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Column 1 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Column 5 UP")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Column 4 UP")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")  // Used. Masked 0x40. See code at PC=0338. (SW2 to enter setup)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,6),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_avt )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
GFXDECODE_END

/*********************************************
*              Machine Drivers               *
*********************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc0" },
	{ "pio1" },
	{ "pio0" },
	{ nullptr }
};

// our pio cannot detect a static interrupt, so we push the current switch state in, then it will work
void avt_state::avtbingo_w(int state)
{
	if (state)
		m_pio0->port_b_write(ioport("IN0")->read());
}

void avt_state::avt(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CPU_CLOCK); /* guess */
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &avt_state::avt_map);
	m_maincpu->set_addrmap(AS_IO, &avt_state::avt_portmap);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();  /* 240x224 (through CRTC) */
	screen.set_screen_update(FUNC(avt_state::screen_update_avt));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_avt);
	PALETTE(config, m_palette, FUNC(avt_state::avt_palette), 8*16);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK)); // guess
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set("ctc0", FUNC(z80ctc_device::trg3));
	crtc.out_vsync_callback().append(FUNC(avt_state::avtbingo_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, "aysnd", CPU_CLOCK/2).add_route(ALL_OUTPUTS, "mono", 1.00);    /* 1.25 MHz.?? */

	// device never addressed by cpu
	z80ctc_device& ctc(Z80CTC(config, "ctc0", CPU_CLOCK)); // U27
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set("ctc0", FUNC(z80ctc_device::trg1));
	// ZC1 not connected
	// TRG2 to TP18; ZC2 to TP9; TRG3 to VSYNC; TRG0 to cpu_clock/4

	Z80PIO(config, m_pio0, CPU_CLOCK); // U23
	m_pio0->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio0->in_pb_callback().set_ioport("IN0");
	// PORT A appears to be lamp drivers
	// PORT B d0-d5 = muxed inputs; d6 = SW2 pushbutton; d7 = ?

	Z80PIO(config, m_pio1, CPU_CLOCK); // U22
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// PORT A d0-d7 = TP13,TP12,TP11,TP10,TP8,TP7,TP5,TP3
	// PORT B d0-d7 = "Player2", DCOM, CCOM, BCOM, ACOM, LOCKOUT/TP6, TP4, 50/60HZ (held high, jumper on JP13 grounds it)
	// DCOM,CCOM,BCOM,ACOM appear to be muxes
}

// Leave avtnfl as it was until more is learnt.
void avt_state::avtnfl_w(int state)
{
	m_pio1->port_b_write((m_pio1->port_b_read() & 0xbf) | (state ? 0x40 : 0));
}

void avt_state::avtnfl(machine_config &config)
{
	avt(config);

	mc6845_device &crtc(MC6845(config.replace(), "crtc", CRTC_CLOCK)); // guess
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set("ctc0", FUNC(z80ctc_device::trg3));
	crtc.out_vsync_callback().append(FUNC(avt_state::avtnfl_w));
}

/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( avtsym14 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "avt1.41.u38", 0x0000, 0x2000, CRC(af4f696a) SHA1(4e969da7db8929b835f5f237fad3b969d5e39ea2) )
	ROM_LOAD( "avt1.42.u39", 0x2000, 0x2000, CRC(eeefbed5) SHA1(1142279fa4939f26c2386f4a9f880f65bed5ee3e) )
	ROM_LOAD( "avt1.43.u40", 0x4000, 0x2000, CRC(906363fa) SHA1(39582c9cf53782d24af7a43fa285e15e8780080b) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "sym.blu", 0x0000, 0x2000, CRC(ee07339b) SHA1(260ac4739c90efa60597bf815d12fb96cf5391ed) )
	ROM_LOAD( "sym.grn", 0x2000, 0x2000, CRC(1df023ac) SHA1(1919ddb835d525fd1843326de939af74693fc88a) )
	ROM_LOAD( "sym.red", 0x4000, 0x2000, CRC(b7688164) SHA1(bc83af273000019f45e42503f8090c4ee8592ffa) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "symbols", 0x0000, 0x0200, CRC(c1a2ecd9) SHA1(21c7a2599e48fa1efccd4f46cf1c34888add2087) )
ROM_END

ROM_START( avtsym25 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // These 2 roms are from different sets and cannot work together
	ROM_LOAD( "u38-2.51.u38", 0x0000, 0x2000, CRC(230a43df) SHA1(395508d5824b50210d6341b958049da10c067201) )
	ROM_LOAD( "u39-2.52.u39", 0x2000, 0x2000, CRC(a1a8f8f6) SHA1(96798fae534bdef6126eeb3e497fab47a4badae9) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "sym.blu", 0x0000, 0x2000, CRC(ee07339b) SHA1(260ac4739c90efa60597bf815d12fb96cf5391ed) )
	ROM_LOAD( "sym.grn", 0x2000, 0x2000, CRC(1df023ac) SHA1(1919ddb835d525fd1843326de939af74693fc88a) )
	ROM_LOAD( "sym.red", 0x4000, 0x2000, CRC(b7688164) SHA1(bc83af273000019f45e42503f8090c4ee8592ffa) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "symbols", 0x0000, 0x0200, CRC(c1a2ecd9) SHA1(21c7a2599e48fa1efccd4f46cf1c34888add2087) )
ROM_END

ROM_START( avtbingo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "avtab.u38", 0x0000, 0x2000, CRC(db4cde32) SHA1(28b50e85e846f99e4314b2ccad6f55a5a3a9326d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "abu18.grn", 0x0000, 0x2000, CRC(7a7fd4b0) SHA1(7af6e7f96921df8467a92794681bbd0f7278652e) )
	ROM_LOAD( "abu18.blu", 0x2000, 0x2000, CRC(73ea8359) SHA1(58bd2c5ba8edcdf818e4b3d84f7bd05a7a05f76a) )
	ROM_LOAD( "abu16.red", 0x4000, 0x2000, CRC(f823fe1c) SHA1(0b7d943f8daa465afa7790c57526c32b7b22bb71) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bingo.avt",    0x0000, 0x0200, CRC(c1a2ecd9) SHA1(21c7a2599e48fa1efccd4f46cf1c34888add2087) )
	ROM_LOAD( "avtbingo.u34", 0x0200, 0x0200, CRC(9454c3de) SHA1(df05f24e607b7494856e627c9f995ffa0cc043f7) )    /* unknown */
ROM_END

ROM_START( avtnfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u38-14.31", 0x0000, 0x2000, CRC(573a0003) SHA1(901ff091f3ce93735539614c6de2efdfda1e99a3) )
	ROM_LOAD( "u39-14.32", 0x2000, 0x2000, CRC(9f9edfa8) SHA1(aa3b039ee1b7143473c996b49f0a13df18e3732d) )
	ROM_LOAD( "u40-14.33", 0x4000, 0x2000, CRC(205910dd) SHA1(37fee06926e4dcd89ec6390b4914a852f12a9e25) )

	/* GFX from Symbols for debugging purposes.
	   Original graphics are missing.
	*/
	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "avtnfl.blu", 0x0000, 0x2000, BAD_DUMP CRC(ee07339b) SHA1(260ac4739c90efa60597bf815d12fb96cf5391ed) )
	ROM_LOAD( "avtnfl.grn", 0x2000, 0x2000, BAD_DUMP CRC(1df023ac) SHA1(1919ddb835d525fd1843326de939af74693fc88a) )
	ROM_LOAD( "avtnfl.red", 0x4000, 0x2000, BAD_DUMP CRC(b7688164) SHA1(bc83af273000019f45e42503f8090c4ee8592ffa) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "avtnfl", 0x0000, 0x0200, CRC(ac975c82) SHA1(9d124115cd7905482bc197462b65d3b5afdab99b) )
ROM_END

} // Anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     STATE       INIT        ROT   COMPANY                      FULLNAME             FLAGS */
GAME( 1985, avtsym14, 0,        avt,      symbols,  avt_state,  empty_init, ROT0, "Advanced Video Technology", "Symbols (ver 1.4)", MACHINE_NOT_WORKING )
GAME( 1985, avtsym25, avtsym14, avt,      symbols,  avt_state,  empty_init, ROT0, "Advanced Video Technology", "Symbols (ver 2.5)", MACHINE_NOT_WORKING )
GAME( 1985, avtbingo, 0,        avt,      avtbingo, avt_state,  empty_init, ROT0, "Advanced Video Technology", "Arrow Bingo",       MACHINE_NOT_WORKING )
GAME( 1989, avtnfl,   0,        avtnfl,   symbols,  avt_state,  empty_init, ROT0, "Advanced Video Technology", "NFL (ver 109)",     MACHINE_NOT_WORKING )
