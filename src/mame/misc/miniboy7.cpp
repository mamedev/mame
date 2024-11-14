// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  MINI-BOY 7 / SUPER MINI-BOY.
  Bonanza Enterprises, Ltd.

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Mini-Boy 7,      1983,  Bonanza Enterprises, Ltd.
  * Super Mini-Boy,  1984,  Bonanza Enterprises, Ltd.


*******************************************************************************

  Game Notes:

  * Mini-Boy 7.

  Seven games in one, plus Ad message support.
  http://www.arcadeflyers.com/?page=thumbs&db=videodb&id=4275

  - Draw Poker.
  - 7-Stud Poker.
  - Black Jack.
  - Baccarat.
  - Hi-Lo.
  - Double-Up.
  - Craps.

  During attract mode display, pressing the service menu will allow you to
  add a custom ad to scroll during attract mode display. Up to 120 characters


  * Super Mini-Boy

  Five games in one.

  - Poker.
  - Black Jack.
  - Hi-Lo.
  - Golden Fruits.
  - Baccarat.


*******************************************************************************

  Hardware Notes:
  --------------

  * Mini-Boy 7:

  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  - CPU:            1x R6502P.
  - Sound:          1x AY-3-8910.
  - Video:          1x HD46505 HD6845SP.
  - RAM:            4x 6116
  - I/O             1x MC6821 PIA.
  - PRG ROMs:       6x 2764 (8Kb).
  - GFX ROMs:       1x 2732 (4Kb) for text layer.
                    3x 2764 (8Kb) for gfx tiles.

  - Clock:          1x 12.4725 MHz. Crystal.

  - Battery backup: (unknown)

  - 1x normal switch (SW1)
  - 1x 8 DIP switches bank (SW2)
  - 1x 4 DIP switches bank (SW3)

  - 1x 2x28 pins edge connector.
  - 1x 2x20 pins female connector.

  - 2x pots to handle the B-G background color/intensity.


  * Super Mini-Boy, Joker Poker

  Board silkscreened on top:
  be MVX-001-02  ('be' is a Bonanza Enterprises logo).

 +------------------------------------------------------+
 |                                     *CN-HEX          |
 |      6  SW2        MSM2128-15   HD46505SP  JDX 3-92  |
 |      8             MSM2128-15              JDX 2-92  |
 |      2             MSM2128-15              JDX 1-92  |
 |      1             MSM2128-15               *28pin   |
++        8    J1                              *28pin   |
|         9                          R6502AP   *28pin   |
|         1                                    *22pin   |
|         0                                    *22pin   |
|   L        JDX                                        |
|   M                                                   |
|   3  VR2              JDX 0                           |
|   8  VR4              JDX 1                           |
|   0  VR5  SW3         JDX 2                           |
|      VR6              JDX 3                           |
++ VR1  VR3   TC4020                              SW1   |
 |     NE555P TC4020   10MHz                        BAT |
 +------------------------------------------------------+

    CPU: Rockwell R6502P
  Video: Hitachi HD46505SP CRT Controller (CRTC) 1MHz
  Sound: AY-3-8910
         LM380N 2.5W Amp
    OSC: 10.000MHz
    RAM: MSM2128-15 2KBx8 SRAM x 4
    DSW: 1 8-switch dipswitches
         1 4-switch dipswitches
VR1-VR6: variable resistor to handle B-G background color/intensity & sound
  Other: Motorola MC6821P 1MHz NMOS Peripheral Interface Adapter (PIA)
         NE555P Texas Instruments Precision Timer
         TC4020BP Toshiba 14 stage ripple carry binary counter
         3.6v Battery
         SW1 - Reset switch
         56 pin edge connector
         CN-HEX 40 pin edge connector

* Denotes unpopulated

  - PRG ROMs:       5x 2764 (8Kb) - 3 for Joker Poker
  - GFX ROMs:       1x 2764 (8Kb) for text layer.
                    3x 2764 (8Kb) for gfx tiles.

NOTE: 10MHz XTAL verified for Joker Poker, Super Mini-Boy is stated as 12.4725MHz

*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  $0000 - $00FF   RAM     ; Zero Page (pointers and registers)
  $0100 - $01FF   RAM     ; 6502 Stack Pointer.
  $0200 - $07FF   RAM     ; R/W. (settings)

  $0800 - $0FFF   Video RAM A
  $1000 - $17FF   Color RAM A
  $1800 - $1FFF   Video RAM B
  $2000 - $27FF   Color RAM B

  $2800 - $2801   MC6845  ; $2800 for register addressing & $2801 for register values.

  $3000 - $3001   AY-3-8910   ; R/W.
  $3080 - $3083   MC6821 PIA  ; R/W.
  $3800 - $3800   ?????       ; R.

  $4000 - $FFFF   ROM     ; ROM space.


  *** mc6845 init ***
  register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
  value:     0x2F  0x25  0x28  0x44  0x27  0x06  0x25  0x25  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.


*******************************************************************************

  DRIVER UPDATES:


  [2007-06-19]

  - Initial release. Just a skeleton driver.


  [2007-06-20]

  - Confirmed the CPU as 6502.
  - Confirmed the CRT controller as 6845.
  - Corrected the total & visible area analyzing the 6845 registers.
  - Crystal documented via #define.
  - CPU clock derived from #defined crystal value.
  - Decoded all gfx properly.
  - Partially worked the GFX banks:
      - 2 bank (1bpp) for text layers and minor graphics.
      - 1 bank (3bpp) for cards, jokers, dices and big text graphics.


  [2010-07-30]

  - Added a new complete set. Now set as parent.
  - Corrected Xtal frequency.
  - Mapped the PIA MC6821 (not wired since is not totally understood).
  - Preliminary attempt to decode the color PROM.
  - Mapped the AY-3-8910, but still needs ports and some checks.
  - Added debug and technical notes.


  [2011... 2019]

  - Wired PIA and AY8910 properly.
  - Implemented and documented the PIA port B multiplexion.
  - Lot of fixes, getting Mini-Boy 7 working.
  - Added support for Super Mini-Boy.
  - Added technical and games notes.
  - Some clean-up.


  TODO:

  - Find the Super Mini-Boy missing input(s): HOLD 1 and SMALL.
  - Find & map 4 switch DSW
    maybe these are the same line/button.
  - Lamps support for Super Mini-Boy.
  - Implement fake pots for B-G background color


*******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "miniboy7.lh"


namespace {

#define MASTER_CLOCK    XTAL(12'472'500)    /* 12.4725 MHz */


class miniboy7_state : public driver_device
{
public:
	miniboy7_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram_a(*this, "videoram_a"),
		m_colorram_a(*this, "colorram_a"),
		m_videoram_b(*this, "videoram_b"),
		m_colorram_b(*this, "colorram_b"),
		m_gfx1(*this, "gfx1"),
		m_gfx2(*this, "gfx2"),
		m_proms(*this, "proms"),
		m_input2(*this, "INPUT2"),
		m_dsw2(*this, "DSW2"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void miniboy7(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void ay_pa_w(uint8_t data);
	void ay_pb_w(uint8_t data);
	uint8_t pia_pb_r();
	void pia_ca2_w(int state);
	uint8_t lamp_latch_r();

	int get_color_offset(uint8_t tile, uint8_t attr, int ra, int px);
	MC6845_UPDATE_ROW(crtc_update_row);
	void miniboy7_palette(palette_device &palette) const;

	void miniboy7_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram_a;
	required_shared_ptr<uint8_t> m_colorram_a;
	required_shared_ptr<uint8_t> m_videoram_b;
	required_shared_ptr<uint8_t> m_colorram_b;
	required_region_ptr<uint8_t> m_gfx1;
	required_region_ptr<uint8_t> m_gfx2;
	required_region_ptr<uint8_t> m_proms;
	required_ioport m_input2;
	required_ioport m_dsw2;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<5> m_lamps;

	uint8_t m_ay_pa;
	uint8_t m_ay_pb;
	int m_gpri;
};


/***********************************
*          Video Hardware          *
***********************************/

int miniboy7_state::get_color_offset(uint8_t tile, uint8_t attr, int ra, int px)
{
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- -xxx   tiles bank.
    ---- x---   seems unused. */

	int color = (attr >> 4) & 0x0f;

	if (attr & 0x04)
	{
		int bank = (attr & 0x03) << 8;
		uint8_t bitplane0 = m_gfx2[0x0000 + ((tile + bank) << 3) + ra];
		uint8_t bitplane1 = m_gfx2[0x2000 + ((tile + bank) << 3) + ra];
		uint8_t bitplane2 = m_gfx2[0x4000 + ((tile + bank) << 3) + ra];

		return (color << 3) + ((BIT(bitplane0 << px, 7) << 0) | (BIT(bitplane1 << px, 7) << 1) | (BIT(bitplane2 << px, 7) << 2));
	}
	else
	{
		int bank = (attr & 0x01) << 8;
		uint8_t bitplane0 = m_gfx1[((tile + bank) << 3) + ra];
		return (color << 3) + BIT(bitplane0 << px, 7);
	}
}

MC6845_UPDATE_ROW( miniboy7_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	for (uint8_t cx = 0; cx < x_count; cx+=1)
	{
		for (int px = 0; px < 8; px++)
		{
			int offset_a = (m_gpri ? 0x80 : 0) + get_color_offset(m_videoram_a[ma + cx], m_colorram_a[ma + cx], ra, px);
			int offset_b = (m_gpri ? 0 : 0x80) + get_color_offset(m_videoram_b[ma + cx], m_colorram_b[ma + cx], ra, px);
			uint8_t color_a = m_proms[offset_a] & 0x0f;
			uint8_t color_b = m_proms[offset_b] & 0x0f;

			if (color_a && (m_gpri || !color_b))          // videoram A has priority
				bitmap.pix(y, (cx << 3) + px) = palette[offset_a];
			else if (color_b && (!m_gpri || !color_a))    // videoram B has priority
				bitmap.pix(y, (cx << 3) + px) = palette[offset_b];
		}
	}
}

void miniboy7_state::miniboy7_palette(palette_device &palette) const
{
	/*
	    prom bits
	    7654 3210
	    ---- ---x   red component?.
	    ---- --x-   green component?.
	    ---- -x--   blue component?.
	    ---- x---   intensity?.
	    xxxx ----   unused.
	*/

	// 0000IBGR
	if (!m_proms)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		int const intenmin = 0xe0;
//      int const intenmin = 0xc2;
		int const intenmax = 0xff;

		// intensity component
		int const inten = BIT(m_proms[i], 3);

		// red component
		int const r = BIT(m_proms[i], 0) * (inten ? intenmax : intenmin);

		// green component
		int const g = BIT(m_proms[i], 1) * (inten ? intenmax : intenmin);

		// blue component
		int const b = BIT(m_proms[i], 2) * (inten ? intenmax : intenmin);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***********************************
*      Machine Start & Reset       *
***********************************/

void miniboy7_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_ay_pa));
	save_item(NAME(m_ay_pb));
	save_item(NAME(m_gpri));
}

void miniboy7_state::machine_reset()
{
	m_ay_pa = 0;
	m_ay_pb = 0;
	m_gpri = 0;
}


/***********************************
*          R/W Handlers            *
***********************************/

void miniboy7_state::ay_pa_w(uint8_t data)
{
/*  ---x xxxx    lamps
    --x- ----    coins lockout
    -x-- ----    coins meter
    x--- ----    unused
*/

	m_ay_pa = data;
}

uint8_t miniboy7_state::lamp_latch_r()
{
	if (machine().side_effects_disabled())
		return 0xff;

	uint8_t data = m_ay_pa ^ 0xff;

	m_lamps[0] = BIT(data, 4);  // [----x]
	m_lamps[1] = BIT(data, 3);  // [---x-]
	m_lamps[2] = BIT(data, 2);  // [--x--]
	m_lamps[3] = BIT(data, 1);  // [-x---]
	m_lamps[4] = BIT(data, 0);  // [x----]

	machine().bookkeeping().coin_counter_w(0, data & 0x40);    // counter

//  popmessage("Out Lamps: %02x", data);
//  logerror("Out Lamps: %02x\n", data);

	// value is unused
	return 0xff;
}

void miniboy7_state::ay_pb_w(uint8_t data)
{
/*  ---- xxxx    unused
    -xxx ----    HCD
    x--- ----    DSW2 select
*/
	m_ay_pb = data;
}

uint8_t miniboy7_state::pia_pb_r()
/*
  PIA PB0-3 are connected to regular inputs.
  AY8910, PortB-D7, is the selector of a 74LS157 that multiplexes the PIA PB4-7 input lines
  to read the 8 DIP switches bank lines. See the following diagram.

                   74LS157
              .-------V-------.
  AY8910 PB7 -|01 SEL    B0 03|- DSW2-8
             -|          B1 06|- DSW2-7
     PIA PB7 -|04 Y0     B3 13|- DSW2-6
     PIA PB6 -|07 Y1     B2 10|- DSW2-5
     PIA PB5 -|12 Y3     A0 02|- DSW2-4
     PIA PB4 -|09 Y2     A1 05|- DSW2-3
             -|          A3 14|- DSW2-2
             -|          A2 11|- DSW2-1
              '---------------'
*/
{
	return (m_input2->read() & 0x0f) | ((m_dsw2->read() << (BIT(m_ay_pb, 7) ? 0 : 4)) & 0xf0);
}

void miniboy7_state::pia_ca2_w(int state)
{
	m_gpri = state;
}


/***********************************
*      Memory Map Information      *
***********************************/

void miniboy7_state::miniboy7_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");  // battery backed RAM?
	map(0x0800, 0x0fff).ram().share("videoram_a");
	map(0x1000, 0x17ff).ram().share("colorram_a");
	map(0x1800, 0x1fff).ram().share("videoram_b");
	map(0x2000, 0x27ff).ram().share("colorram_b");
	map(0x2800, 0x2800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x3000, 0x3001).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x3080, 0x3083).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3800, 0x3800).r(FUNC(miniboy7_state::lamp_latch_r));
	map(0x4000, 0xffff).rom();
}

/*
  ... CRTC init (snap) --> $CF2D: JSR $CF76

  3800: Lamps latch. ; right after each read, another value is loaded
                       into the ACCU, losing the previous loaded value.

*/


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( miniboy7 )
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("INPUT2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("RAM Reset")
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x06, 0x06, "Turns per Coin" )    PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_DIPNAME( 0x18, 0x18, "Bonus Turns" )       PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "50000 100000" )
	PORT_DIPSETTING(    0x10, "100000 200000" )
	PORT_DIPSETTING(    0x08, "100000 300000" )
	PORT_DIPSETTING(    0x00, "200000 300000" )

	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, "Bartop" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2-6" )            PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-7" )            PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2-8" )            PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sminiboy )

//  Hold2/Small button is missing.
//  Just tried on PIA PB4-7, both AY8910 ports,
//  but nothing...

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Big")                    PORT_CODE(KEYCODE_S)  // big
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Cancel / D-UP / Hit")    PORT_CODE(KEYCODE_N)  // cancel / d-up / hit
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hold 5 / Take / Stand")  PORT_CODE(KEYCODE_B)  // hold 5 / take / stand
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hold 4")                 PORT_CODE(KEYCODE_V)  // hold 4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hold 3")                 PORT_CODE(KEYCODE_C)  // hold 3
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Start / Double")       PORT_CODE(KEYCODE_1)  // start
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hold 1 / Play")        PORT_CODE(KEYCODE_Z)  // hold 1 / play
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Service / Books")      PORT_CODE(KEYCODE_0)  // service / books
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("08 - changes screen")  PORT_CODE(KEYCODE_E)  // (changes in screen)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("10")   PORT_CODE(KEYCODE_R)  //
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("20")   PORT_CODE(KEYCODE_T)  //
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("40")   PORT_CODE(KEYCODE_Y)  //
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("80")   PORT_CODE(KEYCODE_U)  //


	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin A" )                    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "1 Point" )
	PORT_DIPSETTING(    0x00, "10.000 Points" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/

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

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), 0 },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/****************************************
*      Graphics Decode Information      *
****************************************/

static GFXDECODE_START( gfx_miniboy7 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 128 )  // text layer

	/*  0x000 cards
	    0x100 joker
	    0x200 dices
	    0x300 bigtxt
	*/
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 32 )

GFXDECODE_END


/***********************************
*         Machine Drivers          *
***********************************/

void miniboy7_state::miniboy7(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK / 16); /* guess */
	m_maincpu->set_addrmap(AS_PROGRAM, &miniboy7_state::miniboy7_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	pia6821_device &pia(PIA6821(config, "pia0"));
	pia.readpa_handler().set_ioport("INPUT1");
	pia.readpb_handler().set(FUNC(miniboy7_state::pia_pb_r));
	pia.ca2_handler().set(FUNC(miniboy7_state::pia_ca2_w));
	pia.irqa_handler().set_inputline("maincpu", 0);
	pia.irqb_handler().set_inputline("maincpu", 0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((47+1)*8, (39+1)*8);             // taken from MC6845, registers 00 & 04 (normally programmed with value - 1).
	screen.set_visarea(0*8, 37*8-1, 0*8, 37*8-1);    // taken from MC6845, registers 01 & 06.
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_miniboy7);

	PALETTE(config, m_palette, FUNC(miniboy7_state::miniboy7_palette), 256);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 12)); /* guess */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(miniboy7_state::crtc_update_row));
	crtc.out_vsync_callback().set("pia0", FUNC(pia6821_device::ca1_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", MASTER_CLOCK / 8));    /* guess */
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.75);
	ay8910.port_a_write_callback().set(FUNC(miniboy7_state::ay_pa_w));
	ay8910.port_b_write_callback().set(FUNC(miniboy7_state::ay_pb_w));
}


/***********************************
*             Rom Load             *
***********************************/

/*
  Mini-Boy 7.
  Board silkscreened on top:
  be MVX-001-01  ('be' is a Bonanza Enterprises logo).

  1x 6502.
  1x AY-3-8910.
  1x MC6821.
  1x HD46505 HD6845SP (Handwritten sticker '107040').
  1x 12.4725 Crystal.

  .a1    2764    No sticker.
  .a3    2764    Stickered 'MB7 5-4'
  .a4    2764    Stickered 'MB7 4-4'
  .a6    2764    Stickered 'MB7 3-4'
  .a7    2764    Stickered 'MB7 2-4'
  .a8    2764    Stickered 'MB7 6-4'
  .d11   2732    Stickered 'MB7 ASC CG'
  .d12   2764    Stickered 'MB7 CG1'
  .d13   2764    Stickered 'MB7 CG2'
  .d14   2764    Stickered 'MB7 CG3'

  .e7    82s10 read as 82s129, stickered 'J'
  .f10   82s10 read as 82s129, stickered 'J'

*/
ROM_START( miniboy7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7_6-4.a8",   0x4000, 0x2000, CRC(a3fdea08) SHA1(2f1a74274005b8c77eb4254d0220206ae4175834) )
	ROM_LOAD( "mb7_2-4.a7",   0x6000, 0x2000, CRC(396e7250) SHA1(8f8c86cc412269157b16ad883638b38bb21345d7) )
	ROM_LOAD( "mb7_3-4.a6",   0x8000, 0x2000, CRC(360a7f7c) SHA1(d98bcfd320680e88b07182d78b4e56fc5579874d) )
	ROM_LOAD( "mb7_4-4.a4",   0xa000, 0x2000, CRC(bff8e334) SHA1(1d09a86b4dbfec6522b326683febaf7426f723e0) )
	ROM_LOAD( "mb7_5-4.a3",   0xc000, 0x2000, CRC(d610bed3) SHA1(67e44ce2345d5429d6ccf4833de207ff6518c534) )
	ROM_LOAD( "nosticker.a1", 0xe000, 0x2000, CRC(5f715a12) SHA1(eabe0e4ee2e110c6ce4fd58c9d36ba80a612d4b5) )    /* ROM 1-4? */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb7_asc_cg.d11", 0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )  /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb7_cg1.d12",    0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )  /* bitplane 1 */
	ROM_LOAD( "mb7_cg2.d13",    0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )  /* bitplane 2 */
	ROM_LOAD( "mb7_cg3.d14",    0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )  /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* both bipolar PROMs are identical */
	ROM_LOAD( "j.e7",   0x0000, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
	ROM_LOAD( "j.f10",  0x0100, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
ROM_END

ROM_START( miniboy7a ) /* The term CREDIT has been changed to POINT is this version, other changes?? */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb7_1-11.a8",  0x4000, 0x2000, CRC(e1c0f8f2) SHA1(0790dc37374cf12313ae13adaea2c6e7338e0dbc) )
	ROM_LOAD( "mb7_2-11.a7",  0x6000, 0x2000, CRC(596040a3) SHA1(bb68b9fd12fba09c3d7c9dec70cf4770d31f911b) )
	ROM_LOAD( "mb7_3-11.a5",  0x8000, 0x2000, CRC(41a9816f) SHA1(9f7853498fcc6ead7cba619421a60335a48dfe57) )
	ROM_LOAD( "mb7_4-11.a4",  0xa000, 0x2000, CRC(bafb08fa) SHA1(004e95a81c94ee40701a604cca4023e6fdece54f) )
	ROM_LOAD( "mb7_5-11.a3",  0xc000, 0x2000, CRC(a8334503) SHA1(ab63f0f602e385445a322663e2e0d6008a25bf5c) )
	ROM_LOAD( "mb7_6-11.a1",  0xe000, 0x2000, CRC(ca9b9b20) SHA1(c6cd793a15948601faa051a4643b14fd3d8bda0b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mb7_0.11d",   0x0000, 0x1000, CRC(84f78ee2) SHA1(c434e8a9b19ef1394b1dac67455f859eef299f95) )  /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mb7_1.12d",   0x0000, 0x2000, CRC(5f3e3b93) SHA1(41ab6a42a41ddeb8b6b76f4d790bf9fb9e7c32a3) )  /* bitplane 1 */
	ROM_LOAD( "mb7_2.13d",   0x2000, 0x2000, CRC(b3362650) SHA1(603907fd3a0049c0a3e1858c4329bf9fd58137f6) )  /* bitplane 2 */
	ROM_LOAD( "mb7_3.14d",   0x4000, 0x2000, CRC(10c2bf71) SHA1(23a01625b0fc0b772054ee4bc026d2257df46a03) )  /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* both bipolar PROMs are identical */
	ROM_LOAD( "j.e7",   0x0000, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
	ROM_LOAD( "j.f10",  0x0100, 0x0100, CRC(4b66215e) SHA1(de4a8f1ee7b9bea02f3a5fc962358d19c7a871a0) ) /* N82S129N BPROM simply labeled J */
ROM_END

/*
  Bonanza's Super Mini Boy.
  PCB: MVX-001-02

  1x 6502
  1x 6845
  1x 6522
  1x AY8910

*/
ROM_START( sminiboy ) /* MVX-001-02 PCB */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm5_1-9.7a",  0x6000, 0x2000, CRC(e245e1d4) SHA1(69266bbc0a0d3acb98cecebf931f42d5e8ff29f5) )
	ROM_LOAD( "sm5_2-9.6a",  0x8000, 0x2000, CRC(0b240c50) SHA1(af53a969d34aaf959b5a8e74f920fe895f6c68a0) )
	ROM_LOAD( "sm5_3-9.4a",  0xa000, 0x2000, CRC(1e389107) SHA1(085f56a544cfec54ea14619af8ae09d7aaf85083) )
	ROM_LOAD( "sm5_4-9.3a",  0xc000, 0x2000, CRC(2fee506a) SHA1(ddcf87ccc061338ca4ccf2f434903ec0c53dee46) )
	ROM_LOAD( "sm5_5-9.1a",  0xe000, 0x2000, CRC(c518b16c) SHA1(3f6249fa40a5e95ad3d565f73cfc45fcad4c5a6e) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sm5_0.11d",   0x0000, 0x2000, CRC(295d8146) SHA1(c5b55e10d04d55ba3a5087b588e697a7a89dd02e) )  /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "sm5_1.12d",   0x0000, 0x2000, CRC(1967974e) SHA1(66e06f549d413dcd84d82245cf3bd918bf012209) )  /* bitplane 1 */
	ROM_LOAD( "sm5_2.13d",   0x2000, 0x2000, CRC(eae3f05a) SHA1(a401fc63f1d9667ceb85c9a8c7cc2bd5d7ab7fcd) )  /* bitplane 2 */
	ROM_LOAD( "sm5_3.15d",   0x4000, 0x2000, CRC(0f19964f) SHA1(2dba35e3770d8254c65ba82b4d062e4873a6fd8f) )  /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* bipolar PROMs */
	ROM_LOAD( "j1.7e",    0x0000, 0x0100, CRC(a89e1d80) SHA1(f3f4842729df1fc379a7281edcceb67c4e9558b4) )
	ROM_LOAD( "j-3.10f",  0x0100, 0x0100, CRC(042ae2c5) SHA1(dad6ace493a2e36c6e8ac1f35e871e1e5071c1ed) )
ROM_END

/*
Same PCB as Super Mini-Boy, but with a 10MHz XTAL installed

Does JDX stand for Joker Draw Extreme (or 10) or something different???

There is no Company or year mentioned or shown during game.
The card backs have the Bonanza Enterprises logo
Bets of 3 to 5 coins adds 1 Joker to the deck.
Bets of 6 to 10 coins adds 1 more Joker to the deck.
Max bet limitted to 10 coins
There is some type of Bonus for 3's or 7's

*/
ROM_START( bejpoker ) /* MVX-001-02 PCB */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jdx_1-92.4a",  0xa000, 0x2000, CRC(40600e8c) SHA1(7f26fec5ccfc99e37c4fcfc7ee25461584af693e) ) /* JDX 1-92, is "92" the year??? */
	ROM_LOAD( "jdx_2-92.3a",  0xc000, 0x2000, CRC(21150010) SHA1(e8757d7a846473232e9bc370a673ddfc36b8ea68) )
	ROM_LOAD( "jdx_3-92.1a",  0xe000, 0x2000, CRC(07497a88) SHA1(2770e77692cb7a0addf80c7955cbb6354e9ed5ec) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "jdx_0.11d",   0x0000, 0x2000, CRC(9a39520c) SHA1(090a94f193b62aa546e5db3399748c298d88794f) ) /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "jdx_1.12d",   0x0000, 0x2000, CRC(c5fe5f09) SHA1(dfa6d486d053e4648471f6b057ea4b2fe01a7348) ) /* bitplane 1 */
	ROM_LOAD( "jdx_2.13d",   0x2000, 0x2000, CRC(cbcf6e85) SHA1(3805d81f41149b61756667470d1df13f691f0ea7) ) /* bitplane 2 */
	ROM_LOAD( "jdx_3.14d",   0x4000, 0x2000, CRC(bf98104d) SHA1(157d71f0a30e15e91f4c8385a7a26af15729c713) ) /* bitplane 3 */

	ROM_REGION( 0x0200, "proms", ROMREGION_INVERT )    /* bipolar PROMs */
	ROM_LOAD( "j1.7e",    0x0000, 0x0100, CRC(a89e1d80) SHA1(f3f4842729df1fc379a7281edcceb67c4e9558b4) ) /* TBP24S10 BPROM (same data as BPROM from Super Mini-Boy) */
	ROM_LOAD( "jdx.10f",  0x0100, 0x0100, CRC(2dd8d3ce) SHA1(6e3d67f9c5ccc210963e02e32fceee4859d8e651) ) /* 63S141 BPROM */
ROM_END

} // anonymous namespace


/***********************************
*           Game Drivers           *
***********************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT        ROT    COMPANY                     FULLNAME             FLAGS                LAYOUT
GAMEL( 1983, miniboy7,  0,        miniboy7, miniboy7, miniboy7_state, empty_init, ROT0, "Bonanza Enterprises, Ltd", "Mini-Boy 7 (set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_miniboy7 )
GAMEL( 1983, miniboy7a, miniboy7, miniboy7, miniboy7, miniboy7_state, empty_init, ROT0, "Bonanza Enterprises, Ltd", "Mini-Boy 7 (set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_miniboy7 )
GAME(  1984, sminiboy,  0,        miniboy7, sminiboy, miniboy7_state, empty_init, ROT0, "Bonanza Enterprises, Ltd", "Super Mini-Boy",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME(  1992, bejpoker,  0,        miniboy7, sminiboy, miniboy7_state, empty_init, ROT0, "Bonanza Enterprises, Ltd", "Bonanza Enterprises' Joker Poker", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
