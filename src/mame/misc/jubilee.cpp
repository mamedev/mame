// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/****************************************************************************************

  Jubilee Double-Up Poker
  -----------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Double-Up Poker (Jubilee),  1985,  Jubilee.


*****************************************************************************************

  You can find more about this game and the emulation evolution at
  http://www.robertofresca.com/

*****************************************************************************************

  Hardware Notes:
  ---------------

  PCB etched:

  HERBER LTD
  Jubilee Sales Pty Ltd.
  BD No V63-261 ISS1.

  1x TMS9980 CPU.
  1x MC6845P CRTC.

  1x TC5517AP-2 (2048 words x 8 bits Asynchronous CMOS Static RAM), tied to a battery.
  1x 2114       (1024 words x 4 bits RAM).

  3x 2732 labelled 1, 2 and 3.
  3x 2764 labelled Red, Blue and Green (the last one also has '22-3-85').

  1x 6.0 MHz crystal.
  1x Panasonic BR-2/3A lithium battery (3V, 1200 mAh).


  From some forums...

  The memory chips on board are the toshiba TC5517ap powered via the Lithium cell and
  the 2114 find the 74148 or 74ls148, all 9980 cpu's use one, pin3 will generate a reset.
  The 5517 ram is compatible with 6116 or 2018.

  The 9980 has 3 interrupt inputs, but they are binary.
  The ls148 encodes the interrupts to the cpu - the highest interrupt is reset.

  I tried pulling pin 3 of the 74ls148 low and yes, this sets up the reset interrupt
  on pins 23, 24 and 25. The TC5517 checks out ok as a 6116.

  The crystal seems ok and this clock makes it throught to pin 34 of the MPU.
  I was expecting that PHASE 03 - pin 22, should be clocking, but it simply sits at 5v.


*****************************************************************************************

  *** Game Notes ***

  Just insert coins, and play. You can win up to 500 credits! :)

  A default NVRAM is provided. Without it, the game stucks at boot with a memory error
  screen. If this happens, press the key 9 (Attendant). The game will initialize the NVRAM
  and then boots OK.

  The RESET service button (key R) is just for clean credits purposes.

  The Supervisor Key (Key 0), brings a menu for bookkeeping, and replay (still can't see
  the point). The key behaves like a real lock key, with off/on status.

  From the marquee: 'ALL CREDITS REDEEMABLE IN LIQUOR ONLY. NO CASH PAYMENTS'
  So we can assume that is *exclusive* for amusement.

  From the marquee: 'MINIMUM REDEMPTION: 30 CREDITS' (to redeem LESS THAN 30 CREDITS,
  Player must insert coins to bring total Credits to 30).

  Ok... you need to have 30 or more credits to redeem. Otherwise you oddly need to insert
  more coins to reach the minimum of 30 Credits.

  To redeem credits (30 or more!):
  1) Press the Hand Pay button (key 8). You'll get the 'Call Attendant' blinking message.
  2) Press the Attendant button (key 9). You'll get a new message at bottom-left of the
     screen about the amount of credits to pay.
  3) Turn ON/OFF the Supervisor key (key 0). It will clear the credits, with a new message
     at bottom-left of the screen about of value paid.


  Paytable:   Play 1 to 5 coins....

  +----------------+--------+--------+--------+--------+--------+
  |     Hand       | 1 Coin | 2 Coin | 3 Coin | 4 Coin | 5 Coin |
  +----------------+--------+--------+--------+--------+--------+
  | Royal Flush    |  100   |  200   |  300   |  400   |  500   |
  | Straight Flush |   70   |  140   |  210   |  280   |  350   |
  | 4 of a Kind    |   40   |   80   |  120   |  160   |  200   |
  | Full House     |   10   |   20   |   30   |   40   |   50   |
  | Flush          |    7   |   14   |   21   |   28   |   35   |
  | Straight       |    5   |   10   |   15   |   20   |   25   |
  | 3 of a Kind    |    3   |    6   |    9   |   12   |   15   |
  | 2 Pair         |    2   |    4   |    6   |    8   |   10   |
  | Pair Aces      |    1   |    2   |    3   |    4   |    5   |
  +----------------+--------+--------+--------+--------+--------+


*****************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0000-2FFF    ; ROM space.
  3000-33FF    ; Video RAM.   ----------> First half of TC5517AP battery backed RAM.
  3400-37FF    ; Working RAM. ----------> Second half of TC5517AP battery backed RAM.
  3800-3BFF    ; Color (ATTR) RAM. -----> Whole 2114 RAM.
  3E00-3E03    ; CRT Controller. -------> MC6845P.

  CRU...

  0080-0080    ; Unknown Read. Maybe a leftover.
  00C8-00C8    ; Multiplexed Input Port.
  0CC2-0CC6    ; Input Port mux selectors.
  0CD0-0CD4    ; Lamps.
  0CE2-0CE2    ; Clear Interrupts line.


  TMS9980A memory map:

  0000-0003 ---> Reset
  0004-0007 ---> Level 1
  0008-000B ---> Level 2
  000C-000F ---> Level 3
  0010-0013 ---> Level 4

  3FFC-3FFF ---> Load


*****************************************************************************************

  DRIVER UPDATES:


  [2014-02-19]

  - Added a default clean NVRAM.
  - Found and implemented the credits input.
    The game is now working!. Still no sound.

  [2014-02-17]

  - Demuxed the input system.
  - Hooked an cleaned all inputs, except the coin in (missing).
  - Added NVRAM support.
  - Hooked CRTC properly.
  - Adjusted the screen size and visible area according to CRTC values.
  - Adjusted the screen pos 8 pixels, to get a bit centered.
  - Fixed palette to 8 colors.
  - Added technical notes.

  [2014-02-17]

  - Corrected the crystal value and derivate clocks via #DEFINE.
  - Improved memory map.
  - Hooked the CRT controller, but the init sequence seems incomplete.
  - Created the accurate graphics banks.
  - Found and mapped the video RAM.
  - Hooked the ATTR RAM.
  - Assigned the correct graphics banks to the proper drawn tiles.
  - Find and mapped an input port.
  - Started a preliminary workaround to demux the input port.
  - Added technical notes.

  [2010-09-05]

  - Initial release.
  - Decoded graphics.
  - Proper tile colors.
  - Hooked the correct CPU.
  - Preliminary memory map.
  - Added technical notes.


  TODO:

  - Analize the remaining CRU writes.
  - Discrete sound?.
  - Check clocks on a PCB (if someday appear one!)


****************************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "machine/nvram.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(6'000'000)              /* confirmed */
#define CPU_CLOCK       MASTER_CLOCK           /* guess */
#define CRTC_CLOCK     (MASTER_CLOCK / 8)      /* guess */


class jubilee_state : public driver_device
{
public:
	jubilee_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoworkram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void jubileep(machine_config &config);

private:
	void jubileep_videoram_w(offs_t offset, uint8_t data);
	void jubileep_colorram_w(offs_t offset, uint8_t data);
	void unk_w(offs_t offset, uint8_t data);
	uint8_t mux_port_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_jubileep(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(jubileep_interrupt);
	void jubileep_cru_map(address_map &map) ATTR_COLD;
	void jubileep_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

	uint8_t mux_sel = 0;
	uint8_t muxlamps = 0;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<9> m_lamps;
};


/*************************
*     Video Hardware     *
*************************/

void jubilee_state::jubileep_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void jubilee_state::jubileep_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(jubilee_state::get_bg_tile_info)
{
/*  - bits -   (attr for gfx banks 00-03)
    7654 3210
    <no> --xx   bank select.
    <no> xx--   seems unused.
*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = (attr & 0x03);
	int color = 0;  /* fixed colors: one rom for each R, G and B. */

	tileinfo.set(bank, code, color, 0);
}


void jubilee_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jubilee_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scrolldx(8, 0); /* guess */
}

uint32_t jubilee_state::screen_update_jubileep(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/**************************
*  Read / Write Handlers  *
**************************/

INTERRUPT_GEN_MEMBER(jubilee_state::jubileep_interrupt)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL1, ASSERT_LINE);
}


/*************************
* Memory Map Information *
*************************/

void jubilee_state::jubileep_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x2fff).rom();

/*  Video RAM =   3000-33FF
    Working RAM = 3400-37FF
    Color RAM =   3800-3BFF (lower 4-bits)
*/
	map(0x3000, 0x37ff).ram().w(FUNC(jubilee_state::jubileep_videoram_w)).share("videoworkram");  /* TC5517AP battery backed RAM */
	map(0x3800, 0x3bff).ram().w(FUNC(jubilee_state::jubileep_colorram_w)).share("colorram");      /* Whole 2114 RAM */

/*  CRTC *is* mapped here. Read 00-01 and then write on them.
    Then does the same for 02-03. Initialization seems incomplete since
    set till register 0x0D. Maybe the last registers are unused.
*/
	map(0x3e00, 0x3e01).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x3e02, 0x3e03).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

/*  CRTC address: $3E01; register: $3E03
    CRTC registers: 2f 20 25 64 26 00 20 23 00 07 00 00 00
    screen total: (0x2f+1)*8 (0x26+1)*(0x07+1) ---> 384 x 312
    visible area: 0x20 0x20  ---------------------> 256 x 256
*/
}


void jubilee_state::unk_w(offs_t offset, uint8_t data)
{
/*  In particular, the interrupt from above must be cleared. We assume that
    this is done by one of the output lines, and from the 32 lines that are
    set right after an interrupt is serviced, all are set to 0, and only one
    is set to one. Maybe this one clears the interrupt.
    TODO: Check the schematics.
*/
	if (((offset<<1)==0x0cf2)&&(data==1))
	{
		m_maincpu->set_input_line(INT_9980A_LEVEL1, CLEAR_LINE);
	}

/*  Inputs Multiplexion...

    Should be 0CC0 added as selector?
    Was tested and didn't see any input driven by its state.
    This doesn't mean that couln't be possible.
*/

	if (((offset<<1)==0x0cc2)&&(data==1))
	{
		mux_sel = 1;
	}

	if (((offset<<1)==0x0cc4)&&(data==1))
	{
		mux_sel = 2;
	}

	if (((offset<<1)==0x0cc6)&&(data==1))
	{
		mux_sel = 3;
	}


/*  Lamps, sounds and other writes:

    0CD0 = HOLD3 lamp.
    0CD2 = HOLD2 lamp.
    0CD4 = HOLD1 lamp.

    Can't find more. Could be a kind of multiplexion.
    The input selectors don't seems to be completely involved,
    but 0CC6 is set to 1 when hold 1-2-3 turn on, thing that
    could be normal due to the end value for the state routine.

    Writes to analize:
    0CC0 (write too often... maybe input selector)
    0CCC (write a bit less often...)

    0CE6 = could be either a 'Insert Coin' lamp,
    or coin lockout (inverted), since is active
    low during the game. when the game is over,
    is set to 1.

    Also...

    Pressing cancel ---> writes 1 to 0CE8
    Pressing bet ------> writes 1 to 0CEA
    Pressing hold4 ----> writes 1 to 0CEC
    Pressing hold5 ----> writes 1 to 0CEE
    Pressing hand pay -> writes 1 to 0CF0
    Pressing hold1 ----> writes 1 to 0CF2
    Pressing hold2 ----> writes 1 to 0CF4
    Pressing hold3 ----> writes 1 to 0CF6
    Pressing reset ----> writes 1 to 0CF8
    Pressing deal -----> writes 1 to 0D00

    (See below, in sound writes...)
*/

	if (((offset<<1)==0x0ccc)&&(data==1))
	{
		muxlamps = 1;
	}
	if (((offset<<1)==0x0ccc)&&(data==0))
	{
		muxlamps = 2;
	}

/*  the following structure has 3 states, because I tested the inputs
    selectors 0CC2-0CC4-0CC6 as lamps multiplexers unsuccessfuly.
*/
	if (((offset<<1)==0x0cd0)&&(data==1))
	{
		if (muxlamps == 1)
			{
				m_lamps[0] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 0 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 0");
			}
		if (muxlamps == 2)
			{
				m_lamps[3] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 3 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 3");
			}
		if (muxlamps == 3)
			{
				m_lamps[6] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 6 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 6");
			}
	}

	if (((offset<<1)==0x0cd2)&&(data==1))
	{
		if (muxlamps == 1)
			{
				m_lamps[1] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 1 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 1");
			}
		if (muxlamps == 2)
			{
				m_lamps[4] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 4 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 4");
			}
		if (muxlamps == 3)
			{
				m_lamps[7] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 7 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 7");
			}
	}

	if (((offset<<1)==0x0cd4)&&(data==1))
	{
		if (muxlamps == 1)
			{
				m_lamps[2] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 2 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 2");
			}
		if (muxlamps == 2)
			{
				m_lamps[5] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 5 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 5");
			}
		if (muxlamps == 3)
			{
				m_lamps[8] = BIT(data, 0);  /* lamp */
				logerror("CRU: LAAAAAAMP 8 write to address %04x: %d\n", offset<<1, data & 1);
//              popmessage("LAMP 8");
			}
	}

/*  Sound writes?...

    In case of discrete circuitry, the following writes
    could be the trigger for each sound.

    Pressing cancel ---> writes 1 to 0CE8
    Pressing bet ------> writes 1 to 0CEA
    Pressing hold4 ----> writes 1 to 0CEC
    Pressing hold5 ----> writes 1 to 0CEE
    Pressing hand pay -> writes 1 to 0CF0
    Pressing hold1 ----> writes 1 to 0CF2
    Pressing hold2 ----> writes 1 to 0CF4
    Pressing hold3 ----> writes 1 to 0CF6
    Pressing reset ----> writes 1 to 0CF8
    Pressing deal -----> writes 1 to 0D00
*/

	if (((offset<<1)==0x0ce8)&&(data==1))
	{
		logerror("CRU: SOUND 'CANCEL' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'CANCEL': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cea)&&(data==1))
	{
		logerror("CRU: SOUND 'BET' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'BET': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cec)&&(data==1))
	{
		logerror("CRU: SOUND 'HOLD4' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HOLD 4': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cee)&&(data==1))
	{
		logerror("CRU: SOUND 'HOLD5' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HOLD 5': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cf0)&&(data==1))
	{
		logerror("CRU: SOUND 'HAND PAY' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HAND PAY': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cf2)&&(data==1))
	{
		logerror("CRU: SOUND 'HOLD1' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HOLD 1': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cf4)&&(data==1))
	{
		logerror("CRU: SOUND 'HOLD2' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HOLD 2': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cf6)&&(data==1))
	{
		logerror("CRU: SOUND 'HOLD3' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'HOLD 3': %04x", offset<<1);
	}

	if (((offset<<1)==0x0cf8)&&(data==1))
	{
		logerror("CRU: SOUND 'RESET' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'RESET': %04x", offset<<1);
	}

	if (((offset<<1)==0x0d00)&&(data==1))
	{
		logerror("CRU: SOUND 'DEAL' write to address %04x: %d\n", offset<<1, data & 1);
//      popmessage("SOUND 'DEAL': %04x", offset<<1);
	}


	/* for debug purposes */

	logerror("CRU write to address %04x: %d\n", offset<<1, data & 1);
}

uint8_t jubilee_state::mux_port_r(offs_t offset)
{
	switch( mux_sel )
	{
		case 0x01: return BIT(ioport("IN0")->read(), offset);
		case 0x02: return BIT(ioport("IN1")->read(), offset);    /* muxed credits input is here! */
		case 0x03: return BIT(ioport("IN2")->read(), offset);
	}

	return 1;
}


void jubilee_state::jubileep_cru_map(address_map &map)
{
	map(0x0c80, 0x0c8f).r(FUNC(jubilee_state::mux_port_r));    /* multiplexed input port */
	map(0x0000, 0x0fff).w(FUNC(jubilee_state::unk_w));
}

/* I/O byte R/W

   0x080  R    ; Input port? Polled once at beginning.
   0x0C8  R    ; Input port.

   Can't see more inputs. There is a multiplexion with the following offsets as selectors:
   0xCC2 / 0xCC4 / 0xCC6.

   0xCC4 status carry the coin input state.

*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jubileep )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )  PORT_NAME("Cancel / Take")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )    PORT_NAME("Bet / Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )   PORT_NAME("Hold 4 / Half Gamble")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )   PORT_NAME("Hold 5 / Red")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_NAME("Hand Pay")             PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )   PORT_NAME("Hold 1 / Black")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )   PORT_NAME("Hold 2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )   PORT_NAME("Hold 3")

	PORT_START("IN1")
/*  Don't know if this needs a custom port. Bits 0 and 1 together are the credits input.
    Bit 1 alone could be "Attendant Paid Status (reset)" that does a reset and update the
    paid status, or just is for edge coin-in timeouts... Impossible to say without a PCB.
*/
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_COIN1 )    PORT_IMPULSE (2)
//  PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Attendant Paid Status (reset)")  PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_CODE(KEYCODE_9) PORT_NAME("Attendant (to pass the memory error and hand pay)")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_CODE(KEYCODE_0) PORT_NAME("Supervisor Key (Bookkeeping)")  PORT_TOGGLE

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )   PORT_NAME("Deal / Start")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	0x100,
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), 0 },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_jubileep )      /* 4 different graphics banks */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, tilelayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, tilelayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, tilelayout, 0, 1 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void jubilee_state::jubileep(machine_config &config)
{
	// Main CPU TMS9980A, no line connections.
	TMS9980A(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &jubilee_state::jubileep_map);
	m_maincpu->set_addrmap(AS_IO, &jubilee_state::jubileep_cru_map);
	m_maincpu->set_vblank_int("screen", FUNC(jubilee_state::jubileep_interrupt));

	NVRAM(config, "videoworkram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);                         /* (47+1*8, 38+1*8) from CRTC settings */
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);   /* (32*8, 32*8) from CRTC settings */
	screen.set_screen_update(FUNC(jubilee_state::screen_update_jubileep));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jubileep);
	PALETTE(config, "palette").set_entries(8);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( jubileep )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "1_ic59.bin", 0x0000, 0x1000, CRC(534c81c2) SHA1(4ce1d4492de9cbbc37e5a946b1183d8e8b0ba989) )
	ROM_LOAD( "2_ic58.bin", 0x1000, 0x1000, CRC(69984028) SHA1(c919a5cb43f23a0d9e496107997c74799709b347) )
	ROM_LOAD( "3_ic57.bin", 0x2000, 0x1000, CRC(c9ae423d) SHA1(8321e3e6fd60d92202b0c7b47e2a333a567b5c22) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ic47.bin",   0x0000, 0x2000, CRC(55dc8482) SHA1(53f22bd66e5fcad5e2397998bc58109c3c19af96) ) /* red */
	ROM_LOAD( "ic48.bin",   0x2000, 0x2000, CRC(a687ec96) SHA1(6a3e0d3796a1505c6d68a9194e9b2b4ef8df5649) ) /* green */
	ROM_LOAD( "ic49.bin",   0x4000, 0x2000, CRC(3e0bc116) SHA1(613c57f0a8baaaa4a04c243a3a139983fa7854e5) ) /* blue */

	ROM_REGION( 0x0800, "videoworkram", 0 )    /* default NVRAM */
	ROM_LOAD( "jubileep_videoworkram.bin", 0x0000, 0x0800, CRC(595bf2b3) SHA1(ae311873b15d8cebfb6ef6a80f27fafc9544178c) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT        ROT   COMPANY    FULLNAME                     FLAGS
GAME( 1985, jubileep, 0,      jubileep, jubileep, jubilee_state, empty_init, ROT0, "Jubilee", "Double-Up Poker (Jubilee)", MACHINE_NO_SOUND )
