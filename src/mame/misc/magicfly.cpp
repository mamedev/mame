// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
// thanks-to:Iris Falbala, Rob Ragon
/******************************************************************************

    MAGIC FLY
    ---------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Magic Fly (P&A Games),          198?
    * 7 e Mezzo (Unknown),            198?
    * Bonne Chance! (French/English), 198?


    **** NOTE ****

    This hardware was clearly designed for poker games.
    You can find a complete hardware & software analysis here:
    http://www.robertofresca.com


    Special Thanks to...

    - EMMA Italian Dumping Team for providing the board.
    - Rob Ragon for the exhaustive hardware testing.
    - Iris Falbala, that kindly offered herself as beta tester,
      poppin' baloons since she was only 2 years old. :)


*******************************************************************************


    *** Hardware notes ***


    - CPU:  1x R6502P.

    - CRTC: 1x MC6845P.

    - RAM:  1x MK48Z02B-20.  CMOS 2K*8 zeropower SRAM (NVRAM).
            2x HM6116LP-4.   2K*8 SRAM (Video RAM, only 1st half used).

    - CLK:  1x crystal @ 10.000 MHz.

    - ROMs: 1x AM27128.      (NS3.1)
            2x SEEQ DQ2764.  (1, 2)
            1x SGS M2764.    (NS1)

    - PLDs: 1x PAL16R4A.     (read protected)

    - SOUND: 1x delta-sigma DAC (1-bit/Bitstream).
             1x TDA2002 (8W. car radio audio amplifier)


    Other components:

    - 3x OUAZ SS-112D  (relay 1 pole 12v)
    - 1x 4 DIP switches
    - 1x 30x2 edge connector
    - 1x 10 pins male connector
    - 2x trimmer (audio?)


    PCB Layout:
    .-----------------------------------------------------------------.
    |                                                                 |
    |                                                                 |
    |            POWER                  .---------. .---------.       |
    |               SUPPLY              | 74LS08N | | 74LS32  |       |
    |                                   '---------' '---------'       |
    |                                   .---------. .---------.       |
    |                                   | 74LS138 | | 74HC00  |       |
    |                                   '---------' '---------'       |
    |   .--------------.               .----------------------.       |
    |   | MK48Z02B-20  |               |       R6502P         |  .----'
    |   |              |               |                      |  |
    |   '--------------'               '----------------------'  |
    | .----------------. .---------.   .----------------------.  |
    | |    AM27128     | | 74LS157 |   |       MC6845P        |  '----.
    | |                | '---------'   |                      |   ----|
    | '----------------' .---------.   '----------------------'   ----|
    |                    | 74LS157 |     .--------. .---------.   ----|
    |                    '---------'     | 74LS14 | | 74LS374 |   ----|
    | .------------.     .---------.     '--------' '---------'   ----|
    | | 74LS245    |     | 74LS157 |                .---------.   ----|
    | '------------'     '---------'                | 74HC244 |   ----|
    | .------------.     .---------.                '---------'   ----|
    | | 74LS245    |     | 74LS32  |     .-------.                ----| 30x2
    | '------------'     '---------'     | | | | |                ----| connector
    | .--------------.                   |4|3|2|1|                ----|
    | | HM6116       |                   | | | | |                ----|
    | | o MSM2128    |                   '-------'                ----|
    | '--------------'                   DIP SW x4                ----|
    | .--------------.                                            ----|
    | | HM6116       |   .--------.     .---------.               ----|
    | | o MSM2128    |   | 74LS08 |     | 74LS174 |               ----|
    | '--------------'   '--------'     '---------'               ----|
    | .----------------. .----------.         .------.            ----|
    | |      2764      | | PAL16R4A |         | TDA  |            ----|
    | |                | '----------'         | 2002 |            ----|
    | '----------------' .----------.         '------'            ----|
    | .----------------. | 74LS166  |    .---.                   .----'
    | |      2764      | '----------'   / POT \                  |
    | |                | .----------.   \     /                  |
    | '----------------' | 74LS166  |    '---'                   '----.
    | .----------------. '----------'                              .--|
    | |      2764      | .----------.    .---------.        .------|  |
    | |                | | 74LS166  |    | 74LS05  |  .---. |SS112D|8 | 10
    | '----------------' '----------'    '---------' / POT \|------|8 | pins
    | .--------..------. .----------.    .---------. \     /|SS112D|8 | male
    | | 74LS04 ||10 MHz| | 74LS193  |    | 74LS86  |  '---' |------|8 | connector
    | '--------'| XTAL | '----------'    '---------'        |SS112D|8 |
    |           '------'                                    '------|  |
    |                                                              '--|
    '-----------------------------------------------------------------'


    Pinouts (from almost unreadable 7mezzo pinout sheet + PCB trace)
    ----------------------------------------------------------------

    *********** Edge connector ************

       Solder side  |Conn|  Components side
    ----------------+----+---------------------
               GND  | 30 |  GND
          +10V. AC  | 29 |  +10V. AC
          +10V. AC  | 28 |  +10V. AC
            unused  | 27 |  unused
            unused  | 26 |  unused
               GND  | 25 |  GND
          +12V. AC  | 24 |  +12V. AC
          +12V. AC  | 23 |  +12V. AC
            unused  | 22 |  unused
      Common C (3)  | 21 |  Common A (1)
      Common D (4)  | 20 |  Common B (2)
              Deal  | 19 |  Double
            Hold 1  | 18 |  Cancel
            Hold 2  | 17 |  Hold 5
            Hold 3  | 16 |  Hold 4
            Meters  | 15 |  Bet
            Coupon  | 14 |
                    | 13 |  Coin 1
      (unreadable)  | 12 |  Coin 2
              Take  | 11 |  Payout
     Small (play1)  | 10 |  Big (play3)
            unused  | 09 |  unused
            unused  | 08 |  unused
            unused  | 07 |  unused
             Green  | 06 |  Red
              Sync  | 05 |  Blue
               GND  | 04 |  GND
          Speaker+  | 03 |  Speaker+
    Speaker- (GND)  | 02 |  Speaker- (GND)
              +5V.  | 01 |  +5V.

    (1) = Double, Deal, Cancel, Bet, Meters.
    (2) = Take, Small, Big, Pay.
    (3) = Hold 1, Hold 2, Hold 3, Hold 4, Hold 5.
    (4) = Coin 1, Coin 2, Coupon.

    Note: Each Common GND (A-B-C-D) are for their respective
    multiplexed groups of inputs, since there are 4 groups
    with 5 valid inputs each one.


    **** 10-Pins connector ****

    pin 01: (soldered to pin 05)
    pin 02:
    pin 03:
    pin 04:
    pin 05: (soldered to pin 01)
    pin 06: (unreadable)
    pin 07: (unreadable)
    pin 08: (unreadable)
    pin 09: (unreadable)
    pin 10: common (GND)


*******************************************************************************


    Memory Map (magicfly)
    ---------------------

    $0000 - $00FF    RAM    ; Zero page (pointers and registers) (NVRAM).

        ($0D)            ; Incremented each time a NMI is triggered.
        ($1D)            ; In case of 0x00, NMI do nothing and return.
        ($11)            ; Store lengths for text handling.
        ($12)            ; Store values to be written in color RAM.
        ($13 - $14)      ; Pointer to text offset.
        ($15 - $16)      ; Pointer to video RAM.
        ($17 - $18)      ; Pointer to color RAM.
        ($19)            ; Program loops waiting for a value to be written here through NMI (see code at $CA71).
        ($1E - $1F)      ; Decrement. Looks as a sort of counter to temporary freeze graphics.
        ($22 - $25)      ; 4-bytes buffer to compare/operate with other segments.
        ($2E - $31)      ; 4-bytes buffer to compare/operate with other segments.
        ($32 - $35)      ; 4-bytes buffer to compare/operate with other segments.
        ($39)            ; Store bits 4, 5 and 6 of content to be ORed with $94 and written to input selector at $3000.
        ($3A - $3D)      ; Store new read values from $2800 (input port) through NMI.
        ($3F - $42)      ; Store old read values from $2800 (input port) through NMI.
        ($56 - $58)      ; Store credits value (shown in the game under "Secondi").
        ($5A - $5C)      ; Store bonus value (shown in the game under "Credits").
        ($5E - $5F)      ; Store values to be written in video and color ram, respectively.
        ($63 - $63)      ; Store position of marker/cursor (0-4).
        ($66 - $66)      ; Store number of baloon to be risen.
        ($67 - $67)      ; Program compare the content with 0x02, 0x03, 0x04, 0x05 and 0xE1.
                         ; If 0xE1 is found here, the machine hangs showing "I/O ERROR" (see code at $C1A2).
        ($6A - $6B)      ; Balloon #1 secondi (bet).
        ($6C - $6D)      ; Balloon #2 secondi (bet).
        ($6E - $6F)      ; Balloon #3 secondi (bet).
        ($70 - $71)      ; Balloon #4 secondi (bet).
        ($72 - $73)      ; Balloon #5 secondi (bet).
        ($94)            ; Store bit 7 of contents to be ORed with $39 and written to output port ($3000).
        ($96 - $98)      ; Store values from content of $2800 (input port) & 0x80, & 0x40, & 0x10.
        ($9B - $A8)      ; Text scroll buffer.

    $0100 - $01FF    RAM    ; 6502 Stack Pointer (NVRAM).

    $0200 - $07FF    RAM    ; General purpose RAM (NVRAM).

    $0800 - $0801    MC6845 ; MC6845 use $0800 for register addressing and $0801 for register values (see code at $CE86).

                     *** MC6845 init ***

                     Register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                     Value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1D  0x1E  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

    $1000 - $13FF    Video RAM  ; Initialized in subroutine starting at $CF83, filled with value stored in $5E.

    $1800 - $1BFF    Color RAM  ; Initialized in subroutine starting at $CF83, filled with value stored in $5F.
                                ; (In 7mezzo is located at $CB13 using $64 and $65 to store video and color ram values.)

    $2800 - $2800    Input port ; Multiplexed input port (code at $CE96).
                                ; NMI routine read from here and store new values in $3A-$3D and copy old ones to $3F-$42.
                                ; Code accept only bits 0-1-2-3-5 as valid inputs. For DIP switches, only bits 4-6-7 are valid.
                                ; If an invalid bit is activated, it will produce an I/O error. (code at $CD0C)

    $3000 - $3000    Output port:

                         Input selector,  ; NMI writes the lower 4 bits to select input.
                         Counters.        ; Bits 4-5-6 are used for Coin1, Coin2, and Payout counters.
                         Sound DAC,       ; Bit 7 is used to transmit DAC data.

    $C000 - $FFFF    ROM space       ; Program ROM.


*******************************************************************************


    After check the last bit of $1800, the code jumps into a loop ($DA30)...

    BEHAVIOUR OF BOOT CHECK (magicfly):

    1) Fills the video RAM with spaces (0x20), and color RAM with 0x15.
    2) Checks bit 7 of $1800 (video RAM, 1st offset) if it's active.
    3) If true, go to $DA30 (incremented fill infinite loop).
    4) If not, fills the video RAM with spaces (0x20), and color RAM with 0x1F.
    5) Checks bit 7 of $1800 (video RAM, 1st offset) if it's active.
    6) If not, go to $DA30 (incremented fill infinite loop).
    7) If true, returns and continues to NORMAL GAME.

    Since bits 0-2 are for regular colors, seems that bit 3 in color RAM
    (bit 2 for 7mezzo) is mirrored to bit 7 through a kind of device.

    This is the only explanation I found to allow a normal boot, and seems to be
    created as a protection method that doesn't allow owners to do a ROM-swap on
    their boards, converting from one game to another.


*******************************************************************************


    Game Notes
    ----------

    Settings (valid for both games):

    Press F2 to enter the input test (game should be without credits).
    In the input test, you can exit with BET + DEAL, or access the bookkeeping
    mode pressing BET + DEAL again. In the bookkeping mode keep pressed the PAYOUT
    key and press BET to change the percentage (all books will be erased). Press
    again BET + DEAL to exit to a quick RAM & sound test, and then to game mode.


    * Magic Fly

    How to play...

    Insert coins to get credits.
    Choose a balloon to raise, and select it with SELECT button. Bet on other balloons
    using the SELECT/BET key (to bet 1 credit) or BETx10 (to bet 10 credits). Once done,
    press the DEAL/LAST BET key to start the game. All balloons will explode revealing
    numbers. The last one to explode will be the raised one. If your number(s) are higher
    than the one hidden in the raised balloon, you'll win!

    You can repeat the last bet pressing the DEAL/LAST BET key.


    * 7 e Mezzo

    This game is a sort of blackjack, but using a spanish cards set. The objetive is
    to get 7 and half points (as 21 in blackjack). If you got more than 7 and half,
    you're busted.

    All cards have their own value, except for 10, 11 and 12 that are half point.
    Special hands have their own price.

    Sun 7 + sun king = 100 (by credit).
    Any 7 + any king =  16 (by credit).
    Any 7 and half   =   8 (by credit).
    Pass             =   2 (by credit).


    How to play...

    Insert coins to get credits.
    Bet using the BET button. Press DEAL to get the cards.
    You can hit a new card pressing DEAL, or stop pressing STAND. The dealer will hit
    cards to the maximum allowed. If you beat the dealer, you'll be asked to enter the
    double-up game. Press DOUBLE to go on, or TAKE to get the credits.

    In the double-up game, a covered card should be shown. Press BIG or SMALL to get
    your chance...


    * Bonne Chance!

    This is a French/English poker game and maybe the hardware was meant for it.
    Seems to be the prototype or prequel of the well known Bonanza's 'Golden Poker'
    and 'Jack Potten Poker' (Good Luck!). Also some Cal Omega poker games are based
    on this poker game.

    With the default DIP switches positions, the game is totally in French, and is
    titled 'BONNE CHANCE!'. Turning the 4th DIP switch ON, the game switch to English,
    and the title changes to 'GOOD LUCK!' (as the above mentioned games).

    To enter the test mode, press SERVICE (key 9). You can see an input-test matrix
    to test all the valid inputs. Pressing BET (key M) and START (Key 1) simultaneou-
    sly, you can find the book-keeping screen. Pressing once again both BET + START,
    a little RAM test will start. As soon as it ends, will exit the mode and will be
    back to the game....


*******************************************************************************


    Driver updates:


    [2006-07-07]
    - Initial release.

    [2006-07-11]
    - Corrected the total number of chars to decode by rom.
    - Fixed the graphics offset for the text layer.
    - Adjusted the gfx rom region bounds properly.

    [2006-07-21]
    - Rewrote the technical info.
    - Removed fuse maps and unaccurate things.
    - Added new findings, pinouts, and pieces of code.
    - Confirmed and partially mapped one input port.
    - Added a little patch to pass over some checks (for debugging purposes).

    [2006-07-26]
    - Figured out the MC6845 (mapped at $0800-$0801).
    - Fixed the screen size based on MC6845 registers.
    - Fixed the visible area based on MC6845 registers.
    - Corrected the gfx rom region.
    - Solved the NMI/vblank issue. Now attract works.
    - Changed CPU clock to 625 kHz. (text scroll looks so fast with the former value)
    - Added new findings to the technical notes.
    - Added version/revision number to magicfly.
    - Marked magicfly PAL as NO_DUMP (read protected).
    - Added flags MACHINE_IMPERFECT_GRAPHICS and MACHINE_WRONG_COLORS.

    [2006-08-06]
    - Figured out how the gfx banks works.
    - Fixed the gfx layers.
    - Fixed the gfx decode.
    - Removed flag MACHINE_IMPERFECT_GRAPHICS.

    [2007-05-12]
    - Figured out how the protection works.
    - Removed the hacks/patchs that formerly allow boot the games.
    - Figured out how the buffered inputs works.
    - Demuxed all inputs for both games.
    - Unified the memory maps from both games.
    - Added NVRAM support to both games.
    - Crystal documented via #define.
    - CPU clock derived from #defined crystal value.
    - Changed CPU clock to 833 kHz.
    - Mapped DIP switches.
    - Found the maximum bet DIP switch in magicfly.
    - Removed flag MACHINE_NOT_WORKING for both games.
    - Managed the planes to get the 3bpp GFX colors accurate.
    - Renamed the ROMs acording to pcb pictures and ROM contents.
    - Cleaned up and optimized the driver.
    - Reworked/updated technical notes.

    [2008-03-14]
    - Completed the component list & PCB layout.
    - Added technical references to register $63 (magicfly).
    - Switched crystal to new predefined format.

    [2008-10-25]
    - Added sound support to magicfly and 7mezzo.
    - Hooked coin counters to magicfly and 7mezzo.
    - Inverted the graphics banks to be straight with the hardware accesses.
    - Updated the memory map description and technical notes.
    - Added game notes and documented the test/settings/bookkeeping modes.

    [2011-10-11]
    - Confirmed and fixed CPU clock for magicfly and 7mezzo.
    - Rearranged the graphic ROMs addressing. Splitted both gfx banks.
    - Created and minimized the color palette for both gfx banks.
    - Fixed colors for magicfly and 7mezzo.

    [2013-01-17]
    - Added Bonne Chance!. A French/English poker game prototype of
       the well known 'Golden Poker' and 'Jack Potten Poker'.
    - Worked complete inputs from the scratch. Promoted to working.
    - Added proper palette. Now the game seems to get accurate colors.
    - Added some notes.


    TODO:

    - Simplify the gfx banks to avoid a custom palette.
    - Document the correct pinout.
    - Analyze the PLD. Try to reconstruct the original equations.
    - Split the driver.


*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(10'000'000)


class magicfly_state : public driver_device
{
public:
	magicfly_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void bchance(machine_config &config);
	void magicfly(machine_config &config);
	void _7mezzo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap = nullptr;
	int m_input_selector = 0;
	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

	void magicfly_videoram_w(offs_t offset, uint8_t data);
	void magicfly_colorram_w(offs_t offset, uint8_t data);
	uint8_t mux_port_r();
	void mux_port_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_magicfly_tile_info);
	TILE_GET_INFO_MEMBER(get_7mezzo_tile_info);
	void magicfly_palette(palette_device &palette) const;
	void bchance_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(7mezzo);
	uint32_t screen_update_magicfly(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void magicfly_map(address_map &map) ATTR_COLD;
};


/*********************************************
*               Video Hardware               *
*********************************************/


void magicfly_state::magicfly_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void magicfly_state::magicfly_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(magicfly_state::get_magicfly_tile_info)
{
/*  - bits -
    7654 3210
    ---- -xxx   Tiles color.
    ---- x---   Seems to be a kind of attribute (maybe related to color). Not totally understood yet.
    ---x ----   Tiles bank.
    -xx- ----   Apparently not used.
    x--- ----   Mirrored from bit 3. The code check this one to boot the game.

*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = (attr & 0x10) >> 4;   /* bit 4 switch the gfx banks */
	int color = attr & 0x07;         /* bits 0-2 for color */

	/* Seems that bit 7 is mirrored from bit 3 to have a normal boot */
	/* Boot only check the first color RAM offset */

	m_colorram[0] = m_colorram[0] | ((m_colorram[0] & 0x08) << 4);  /* only for 1st offset */
	//m_colorram[tile_index] = attr | ((attr & 0x08) << 4);         /* for the whole color RAM */

	tileinfo.set(bank, code, color, 0);
}

void magicfly_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magicfly_state::get_magicfly_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 29);
}


TILE_GET_INFO_MEMBER(magicfly_state::get_7mezzo_tile_info)
{
/*  - bits -
    7654 3210
    ---- -xxx   Tiles color.
    ---- x---   Seems to be a kind of attribute (maybe related to color). Not totally understood yet.
    ---x ----   Tiles bank.
    -xx- ----   Apparently not used.
    x--- ----   Mirrored from bit 2. The code check this one to boot the game.

*/
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index];
	int const bank = (attr & 0x10) >> 4;    /* bit 4 switch the gfx banks */
	int const color = attr & 0x07;          /* bits 0-2 for color */

	/* Seems that bit 7 is mirrored from bit 2 to have a normal boot */
	/* Boot only check the first color RAM offset */

	m_colorram[0] = m_colorram[0] | ((m_colorram[0] & 0x04) << 5);  /* only for 1st offset */
	//m_colorram[tile_index] = attr | ((attr & 0x04) << 5);         /* for the whole color RAM */

	tileinfo.set(bank, code, color, 0);
}

VIDEO_START_MEMBER(magicfly_state, 7mezzo)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magicfly_state::get_7mezzo_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 29);
}


uint32_t magicfly_state::screen_update_magicfly(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void magicfly_state::magicfly_palette(palette_device &palette) const
{
	for (int i = 0x00; i < 0x10; i += 0x10)
	{
		/* 1st gfx bank */
		palette.set_pen_color(i + 0, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 2, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 4, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 6, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 8, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 10, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 12, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 14, rgb_t(0x00, 0x00, 0x00));

		palette.set_pen_color(i + 1, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 3, rgb_t(0xff, 0x00, 0x00));
		palette.set_pen_color(i + 5, rgb_t(0x00, 0xff, 0x00));
		palette.set_pen_color(i + 7, rgb_t(0xff, 0xff, 0x00));
		palette.set_pen_color(i + 9, rgb_t(0x00, 0x00, 0xff));
		palette.set_pen_color(i + 11, rgb_t(0xff, 0x00, 0xff));
		palette.set_pen_color(i + 13, rgb_t(0x00, 0xff, 0xff));
		palette.set_pen_color(i + 15, rgb_t(0xff, 0xff, 0xff));
	}
}

void magicfly_state::bchance_palette(palette_device &palette) const
{
	for (int i = 0x00; i < 0x10; i += 0x10)
	{
		/* 1st gfx bank */
		palette.set_pen_color(i + 0, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 2, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 4, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 6, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 8, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 10, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 12, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 14, rgb_t(0x00, 0x00, 0x00));

		palette.set_pen_color(i + 1, rgb_t(0x00, 0x00, 0x00));
		palette.set_pen_color(i + 3, rgb_t(0xff, 0x00, 0x00));
		palette.set_pen_color(i + 5, rgb_t(0x00, 0xff, 0x00));
		palette.set_pen_color(i + 7, rgb_t(0xff, 0xff, 0x00));
		palette.set_pen_color(i + 9, rgb_t(0x00, 0x00, 0xff));
		palette.set_pen_color(i + 11, rgb_t(0xff, 0x00, 0xff));
		palette.set_pen_color(i + 13, rgb_t(0x00, 0xff, 0xff));
		palette.set_pen_color(i + 15, rgb_t(0xff, 0xff, 0xff));
	}

	palette.set_pen_color(0x08 , rgb_t(0xff, 0xff, 0xff));    // white for the cards back logo background.
	palette.set_pen_color(0x12 , rgb_t(0x00, 0x00, 0x00));    // black for the cards corners (should be transparent)
}


/**************************************************
*                   R/W Handlers                  *
**************************************************/


uint8_t magicfly_state::mux_port_r()
{
	switch( m_input_selector )
	{
		case 0x01: return ioport("IN0-0")->read();
		case 0x02: return ioport("IN0-1")->read();
		case 0x04: return ioport("IN0-2")->read();
		case 0x08: return ioport("IN0-3")->read();
		case 0x00: return ioport("DSW0")->read();
	}
	return 0xff;
}

void magicfly_state::mux_port_w(uint8_t data)
{
/*  - bits -
    7654 3210
    ---- xxxx   Input selector.
    ---x ----   Coin 2.
    --x- ----   Payout.
    -x-- ----   Coin 1.
    x--- ----   Sound DAC.

*/
	m_input_selector = data & 0x0f; /* Input Selector */

	m_dac->write(BIT(data, 7));      /* Sound DAC */

	machine().bookkeeping().coin_counter_w(0, data & 0x40);  /* Coin1 */
	machine().bookkeeping().coin_counter_w(1, data & 0x10);  /* Coin2 */
	machine().bookkeeping().coin_counter_w(2, data & 0x20);  /* Payout */
}


/*********************************************
*           Memory map information           *
*********************************************/

void magicfly_state::magicfly_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");    /* MK48Z02B NVRAM */
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1000, 0x13ff).ram().w(FUNC(magicfly_state::magicfly_videoram_w)).share("videoram"); /* HM6116LP #1 (2K x 8) RAM (only 1st half used) */
	map(0x1800, 0x1bff).ram().w(FUNC(magicfly_state::magicfly_colorram_w)).share("colorram"); /* HM6116LP #2 (2K x 8) RAM (only 1st half used) */
	map(0x2800, 0x2800).r(FUNC(magicfly_state::mux_port_r));    /* multiplexed input port */
	map(0x3000, 0x3000).w(FUNC(magicfly_state::mux_port_w));   /* output port */
	map(0xc000, 0xffff).rom();                 /* ROM space */
}


/*********************************************
*                Input ports                 *
*********************************************/

static INPUT_PORTS_START( magicfly )
/*  Multiplexed 4 x 5 bits.
    Code accept only bits 0, 1, 2, 3 and 5 as valid.
*/
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Deal / Last Bet") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Cancel") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet10") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet1/Select") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")    /* Only 4 physical DIP switches (valid bits = 4, 6, 7) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Maximum Bet" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  /* invalid - don't change */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 7mezzo )
/*  Multiplexed 4 x 5 bits.
    Code accept only bits 0, 1, 2, 3 and 5 as valid.
*/
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Big") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Small") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Take") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal / Last Bet") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Double") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")    /* Only 4 physical DIP switches (valid bits = 4, 6, 7) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  /* invalid - don't change */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bchance )
/*  Multiplexed 4 x 5 bits.
    Code accept only bits 0, 1, 2, 3 and 5 as valid.

    Input Test grid...

        C1 C2 C3 C4 C5

    R1  0  0  0  0  0
    R2  0  0  0  0  0
    R3  0  0  0  0  0
    R4  0  0  0  0  0

    R4C1 + R4C5 to exit...
*/
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )                                          // input test R1C1 (coin 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                                          // input test R1C2 (coin 2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN0-3")  // input test R1C3 (unknown)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN0-4")  // input test R1C4 (unknown)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN0-6")  // input test R1C5 (unknown)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )    PORT_NAME("Small")               // input test R2C1 (small)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )   PORT_NAME("Big")                 // input test R2C2 (big)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout")              // input test R2C3 (payout)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                                    // input test R2C4 (take)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN1-6")  // input test R2C5 (unknown)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    // input test R3C1 (hold 4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    // input test R3C2 (hold 5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    // input test R3C3 (hold 2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    // input test R3C4 (hold 3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    // input test R3C5 (hold 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")  // input test R4C1 (start/deal)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )               // input test R4C2 (cancel)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )             // input test R4C3 (service/test)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                // input test R4C4 (d-up)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                 // input test R4C5 (bet)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
/*  Only 4 physical DIP switches
    (valid bits = 4, 6, 7)
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Bet Max" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Language" )
	PORT_DIPSETTING(    0x80, "French" )
	PORT_DIPSETTING(    0x00, "English" )
INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_magicfly )
	GFXDECODE_ENTRY( "gfxbnk1", 0, tilelayout, 16, 1 )
	GFXDECODE_ENTRY( "gfxbnk0", 0, charlayout, 0, 8 )
GFXDECODE_END


/*********************************************
*              Machine Drivers               *
*********************************************/

void magicfly_state::magicfly(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK / 16); /* guess */
	m_maincpu->set_addrmap(AS_PROGRAM, &magicfly_state::magicfly_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((39+1)*8, (31+1)*8);                /* Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1). */
	screen.set_visarea(0*8, 32*8-1, 0*8, 29*8-1);  /* Taken from MC6845 init, registers 01 & 06. */
	screen.set_screen_update(FUNC(magicfly_state::screen_update_magicfly));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_magicfly);
	PALETTE(config, "palette", FUNC(magicfly_state::magicfly_palette), 32);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK/16)); /* guess */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}


void magicfly_state::_7mezzo(machine_config &config)
{
	magicfly(config);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(magicfly_state, 7mezzo)
}


void magicfly_state::bchance(machine_config &config)
{
	magicfly(config);

	/* video hardware */
	subdevice<palette_device>("palette")->set_init(FUNC(magicfly_state::bchance_palette));
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( magicfly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "magicfly3_3.bin",    0xc000, 0x4000, CRC(c29798d5) SHA1(bf92ac93d650398569b3ab79d01344e74a6d35be) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "magicfly2.bin",  0x0000, 0x2000, CRC(3596a45b) SHA1(7ec32ec767d0883d05606beb588d8f27ba8f10a4) )
	ROM_LOAD( "magicfly1.bin",  0x2000, 0x2000, CRC(724d330c) SHA1(cce3923ce48634b27f0e7d29979cd36e7394ab37) )
	ROM_LOAD( "magicfly0.bin",  0x4000, 0x2000, CRC(44e3c9d6) SHA1(677d25360d261bf2400f399b8015eeb529ad405e) )

	ROM_REGION( 0x0800, "gfxbnk0", 0 )
	ROM_COPY( "gfx",    0x1800, 0x0000, 0x0800 )    /* chars */

	ROM_REGION( 0x1800, "gfxbnk1", 0 )
	ROM_COPY( "gfx",    0x1000, 0x0000, 0x0800 )    /* sprites, bitplane 1 */
	ROM_COPY( "gfx",    0x3800, 0x0800, 0x0800 )    /* sprites, bitplane 2 */
	ROM_COPY( "gfx",    0x5800, 0x1000, 0x0800 )    /* sprites, bitplane 3 */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r4a-magicfly.bin",  0x0000, 0x0104, NO_DUMP )   /* PAL is read protected */

ROM_END

ROM_START( 7mezzo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ns3_1.bin",  0xc000, 0x4000, CRC(b1867b76) SHA1(eb76cffb81c865352f4767015edade54801f6155) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "ns2.bin",    0x0000, 0x2000, CRC(7983a41c) SHA1(68805ea960c2738d3cd2c7490ffed84f90da029b) )    /* Renamed as ns2.bin regarding pcb location and content */
	ROM_LOAD( "ns1.bin",    0x2000, 0x2000, CRC(a6ada872) SHA1(7f531a76e73d479161e485bdcf816eb8eb9fdc62) )
	ROM_LOAD( "ns0.bin",    0x4000, 0x2000, CRC(e04fb210) SHA1(81e764e296fe387daf8ca67064d5eba2a4fc3c26) )    /* Renamed as ns0.bin regarding pcb location and content */

	ROM_REGION( 0x0800, "gfxbnk0", 0 )
	ROM_COPY( "gfx",    0x1800, 0x0000, 0x0800 )    /* chars */

	ROM_REGION( 0x1800, "gfxbnk1", 0 )
	ROM_COPY( "gfx",    0x1000, 0x0000, 0x0800 )    /* 3bpp tiles, bitplane 1 */
	ROM_COPY( "gfx",    0x3800, 0x0800, 0x0800 )    /* 3bpp tiles, bitplane 2 */
	ROM_COPY( "gfx",    0x5800, 0x1000, 0x0800 )    /* 3bpp tiles, bitplane 3 */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r4a-7mezzo.bin",    0x0000, 0x0104, BAD_DUMP CRC(61ac7372) SHA1(7560506468a7409075094787182ded24e2d0c0a3) )
ROM_END

ROM_START( bchance )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v-pk-4gag.bin",  0xc000, 0x4000, CRC(7c2dd908) SHA1(97b1390fb4c8c838a0d5b78d6904d597a9abe27f) )

	ROM_REGION( 0x6000, "gfx", 0 )  /* ROM n-pk-2.bin was created from an exhaustive analysis of 25 different bad dumps */
	ROM_LOAD( "n-pk-2.bin", 0x0000, 0x2000, BAD_DUMP CRC(462c3dd7) SHA1(fb30d6147e0d607b3fb631d8bdca35e98eccfd2d) )
	ROM_LOAD( "n-pk-1.bin", 0x2000, 0x2000, CRC(e35cebd6) SHA1(b0dd86fd4c06f98e486b04e09808985bfa4f0e9c) )
	ROM_LOAD( "n-pk-0.bin", 0x4000, 0x2000, CRC(3c64edc4) SHA1(97b677b7c4999b502ab4b4f70c33b40050843796) )

	ROM_REGION( 0x0800, "gfxbnk0", 0 )
	ROM_COPY( "gfx",    0x1800, 0x0000, 0x0800 )    /* chars */

	ROM_REGION( 0x1800, "gfxbnk1", 0 )
	ROM_COPY( "gfx",    0x1000, 0x0000, 0x0800 )    /* 3bpp tiles, bitplane 1 */
	ROM_COPY( "gfx",    0x3800, 0x0800, 0x0800 )    /* 3bpp tiles, bitplane 2 */
	ROM_COPY( "gfx",    0x5800, 0x1000, 0x0800 )    /* 3bpp tiles, bitplane 3 */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-bchance.bin",    0x0000, 0x0104, NO_DUMP )   /* protected */
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT   COMPANY      FULLNAME                          FLAGS
GAME( 198?, magicfly, 0,      magicfly, magicfly, magicfly_state, empty_init, ROT0, "P&A Games", "Magic Fly",                      0 )
GAME( 198?, 7mezzo,   0,      _7mezzo,  7mezzo,   magicfly_state, empty_init, ROT0, "<unknown>", "7 e Mezzo",                      0 )
GAME( 198?, bchance,  0,      bchance,  bchance,  magicfly_state, empty_init, ROT0, "<unknown>", "Bonne Chance! (French/English)", MACHINE_IMPERFECT_GRAPHICS )
