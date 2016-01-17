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

  The winning symbols are selected at random at the start of the game and displayed with a dis-
  tinctive yellow color surrounding that symbol in each column.


  HOW TO PLAY THE GAME:

  The player must first insert coins to obtain points to play the game. Once points are on the
  screen the player may then press the play button one time for each point available up to the
  coin multiplier limit (except when lockouts are in effect) at which time the game will auto
  start. If the player wants to start before the coin max, he may press the start button to i-
  nitiate play. At the start of the game arrows are displayed at the top of columns to show the
  direction to move that column in order to get the winning symbol on screen, in the event that
  particular winning symbol is already on screen, that symbol will appear at the top of the co-
  lumn instead of the arrow. When the player has all the winning symbols on the screen an align-
  ment of winning symbols, either horizontally or diagonally, should be done with the least a-
  mount of moves in the shortest amount of time. In the event the lockout mode is engaged the
  game will accept coins up to the lockout limit and will then auto start.

  The game has two different win modes defined as follows. If an alignment is obtained in 13
  moves or less, the player will be awarded points defined by the award tables. If alignment
  is obtained from 14 to 32, moves stars will then be awarded for the amount of moves as de-
  fined by the awards table. A bonus of 3 stars will be awarded if game is completed in 3 se-
  conds or less and 100 stars equals 1 point.

  Time is also an element in this game. When the game is started the player has a specific a-
  mount of time to complete the game, also the arrows have a time limit. These time factors
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
  on your front panel. To exit a mode just depress the START button on your front panel. To ad-
  just an entry in one of the modules use the second up or down arrow on your front panel. Now
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

  To change PLAY TIME SETUP, enter this module and you may change PLAY TIME or ARROW TIME (the a-
  mount of time arrows are allowed to stay on screen during play). To enable the lockout mode, you
  may set the lockout number to 1 or 2 or 3 or 4 and 0 if no lockout is desired. To disable the
  STARS set the STAR MODE to OFF. To adjust the UP AWARDS set to ON for more upper awards and to
  OFF for less upper awards table wins.

  The default name used in BUSINESS NAME SETUP is "THE MANAGEMENT". If you want to put a location
  name in the attract mode, enter the business name setup and use the second up and down arrows to
  select from the alphabet. In the event you make a mistake you can backup by selecting the first
  up arrow. When you are finished with your message just exit by pressing START button and the me-
  ssage will automatically center when you exit.


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


************************************************************************************************/


#define MASTER_CLOCK    XTAL_10MHz          /* unknown */
#define CPU_CLOCK       MASTER_CLOCK/4      /* guess... seems accurate */
#define CRTC_CLOCK      MASTER_CLOCK/16     /* it gives 59.410646 fps with current settings */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
//#include "machine/z80ctc.h"
//#include "machine/z80pio.h"


class avt_state : public driver_device
{
public:
	avt_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_crtc(*this, "crtc"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(avt_6845_address_w);
	DECLARE_WRITE8_MEMBER(avt_6845_data_w);
	DECLARE_READ8_MEMBER( avt_6845_data_r );
	DECLARE_WRITE8_MEMBER(avt_videoram_w);
	DECLARE_WRITE8_MEMBER(avt_colorram_w);

	tilemap_t *m_bg_tilemap;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(avt);
	UINT32 screen_update_avt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(avt_vblank_irq);
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


WRITE8_MEMBER( avt_state::avt_videoram_w )
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER( avt_state::avt_colorram_w )
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

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


void avt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(avt_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 28, 32);
}


UINT32 avt_state::screen_update_avt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	count = 0;

	for(y=0;y<mc6845_v_display;y++)
	{
		for(x=0;x<mc6845_h_display;x++)
		{
			UINT16 tile = m_videoram[count] | ((m_colorram[count] & 1) << 8);
			UINT8 color = (m_colorram[count] & 0xf0) >> 4;

			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*8,(y*8));

			count++;
		}
	}
	//m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


PALETTE_INIT_MEMBER(avt_state, avt)
{
	const UINT8 *color_prom = memregion("proms")->base();
/*  prom bits
    7654 3210
    ---- ---x   Intensity?.
    ---- --x-   Red component.
    ---- -x--   Green component.
    ---- x---   Blue component.
    xxxx ----   Unused.
*/
	int j;

	/* 0000BGRI */
	if (color_prom == nullptr) return;

	for (j = 0; j < palette.entries(); j++)
	{
		int bit1, bit2, bit3, r, g, b, inten, intenmin, intenmax, i;

		intenmin = 0xe0;
		intenmax = 0xff;

		i = ((j & 0x7) << 4) | ((j & 0x78) >> 3);


		/* intensity component */
//      inten = 1 - (color_prom[i] & 0x01);
		inten = (color_prom[i] & 0x01);

		/* red component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		r = (bit1 * intenmin) + (inten * (bit1 * (intenmax - intenmin)));

		/* green component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		g = (bit2 * intenmin) + (inten * (bit2 * (intenmax - intenmin)));

		/* blue component */
		bit3 = (color_prom[i] >> 3) & 0x01;
		b = (bit3 * intenmin) + (inten * (bit3 * (intenmax - intenmin)));


		/* hack to switch cyan->magenta for highlighted background */
		if (j == 0x40)
			palette.set_pen_color(j, rgb_t(g, r, b)); // Why this one has R-G swapped?...
		else
			palette.set_pen_color(j, rgb_t(r, g, b));
	}
}


/**********************************************
*            Read / Write Handlers            *
**********************************************/

//WRITE8_MEMBER(avt_state::debug_w)
//{
//  popmessage("written : %02X", data);
//}

WRITE8_MEMBER( avt_state::avt_6845_address_w )
{
	m_crtc_index = data;
	m_crtc->address_w(space, offset, data);
}

WRITE8_MEMBER( avt_state::avt_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(space, offset, data);
}

READ8_MEMBER( avt_state::avt_6845_data_r )
{
	//m_crtc_vreg[m_crtc_index] = data;
	return m_crtc->register_r(space, offset);
}

/*********************************************
*           Memory Map Information           *
*********************************************/

/* avtnfl, avtbingo */
static ADDRESS_MAP_START( avt_map, AS_PROGRAM, 8, avt_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_RAM // AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xa7ff) AM_RAM_WRITE(avt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(avt_colorram_w) AM_SHARE("colorram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( avt_portmap, AS_IO, 8, avt_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x03) unk, maybe IO
//  AM_RANGE(0x00, 0x00)  AM_READ_PORT("IN0")
//  AM_RANGE(0x01, 0x01)  AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW1")
//  AM_RANGE(0x08, 0x0b) unk, maybe IO
//  AM_RANGE(0x08, 0x08)  AM_READ_PORT("IN2")
//  AM_RANGE(0x09, 0x09)  AM_READ_PORT("IN3")
	AM_RANGE(0x21, 0x21) AM_DEVWRITE("aysnd", ay8910_device, data_w)     /* AY8910 data */
	AM_RANGE(0x23, 0x23) AM_DEVWRITE("aysnd", ay8910_device, address_w)      /* AY8910 control */
	AM_RANGE(0x28, 0x28) AM_WRITE(avt_6845_address_w)
	AM_RANGE(0x29, 0x29) AM_READWRITE(avt_6845_data_r,avt_6845_data_w)
ADDRESS_MAP_END

/* I/O byte R/W


   -----------------

   unknown writes:




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

static GFXDECODE_START( avt )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
GFXDECODE_END

/*********************************************
*              Machine Drivers               *
*********************************************/

/* IM 2 */
INTERRUPT_GEN_MEMBER(avt_state::avt_vblank_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x06);
}

static MACHINE_CONFIG_START( avt, avt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK) /* guess */
	MCFG_CPU_PROGRAM_MAP(avt_map)
	MCFG_CPU_IO_MAP(avt_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", avt_state,  avt_vblank_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)  /* 240x224 (through CRTC) */
	MCFG_SCREEN_UPDATE_DRIVER(avt_state, screen_update_avt)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", avt)

	MCFG_PALETTE_ADD("palette", 8*16)
	MCFG_PALETTE_INIT_OWNER(avt_state, avt)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)    /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, CPU_CLOCK/2)    /* 1.25 MHz.?? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


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
	ROM_REGION( 0x10000, "maincpu", 0 )
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


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     INIT  ROT    COMPANY                      FULLNAME            FLAGS */
GAME( 1985, avtsym14, 0,        avt,      symbols, driver_device,  0,    ROT0, "Advanced Video Technology", "Symbols (ver 1.4)", MACHINE_NOT_WORKING )
GAME( 1985, avtsym25, avtsym14, avt,      symbols, driver_device,  0,    ROT0, "Advanced Video Technology", "Symbols (ver 2.5)", MACHINE_NOT_WORKING )
GAME( 1985, avtbingo, 0,        avt,      symbols, driver_device,  0,    ROT0, "Advanced Video Technology", "Arrow Bingo",       MACHINE_NOT_WORKING )
GAME( 1989, avtnfl,   0,        avt,      symbols, driver_device,  0,    ROT0, "Advanced Video Technology", "NFL (ver 109)",     MACHINE_NOT_WORKING )
