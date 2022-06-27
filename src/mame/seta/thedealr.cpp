// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    The Dealer (Visco Games)

    Driver by Luca Elia
    This game runs on Seta Hardware

    P0-040A PCB:

    R65C02P2 x 2
    X0-009 (Intel 8742 MCU)

    X1-001
    X1-002
    X1-003
    X1-004
    X1-006

    Yamaha YM2149F

    MC68B50P ACIA

    DSW8 x 4, Reset Button
    CR2032 3V Battery
    XTAL 16MHz

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "seta001.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class thedealr_state : public driver_device
{
public:
	thedealr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_seta001(*this, "spritegen"),
		m_palette(*this, "palette"),
		m_iox_io(*this, "IOX"),
		m_leds(*this, "led%u", 0U)
	{ }

	void thedealr(machine_config &config);

private:
	// IOX
	uint8_t iox_r();
	void iox_w(uint8_t data);
	uint8_t iox_status_r();
	uint8_t m_iox_cmd, m_iox_ret, m_iox_status, m_iox_leds, m_iox_coins;
	void iox_reset();

	// memory map
	uint8_t irq_ack_r();
	void unk_w(uint8_t data);

	// machine
	TIMER_DEVICE_CALLBACK_MEMBER(thedealr_interrupt);

	// video
	void thedealr_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void thedealr_main(address_map &map);
	void thedealr_sub(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<seta001_device> m_seta001;
	required_device<palette_device> m_palette;
	optional_ioport m_iox_io;
	output_finder<8> m_leds;
};

/***************************************************************************

    Video

***************************************************************************/

void thedealr_state::thedealr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) | color_prom[i + 512];
		palette.set_pen_color(i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}

uint32_t thedealr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->draw_sprites(screen, bitmap, cliprect, 0x1000);
	return 0;
}

WRITE_LINE_MEMBER(thedealr_state::screen_vblank)
{
	// rising edge
	if (state)
		m_seta001->setac_eof();
}

/***************************************************************************

    IOX (i8742 MCU) Simulation

***************************************************************************/

#define IOX_OUT_FULL 0x01
#define IOX_IN_FULL  0x02
#define IOX_WAITDATA 0x80

void thedealr_state::iox_reset()
{
	m_iox_status    =   0x00;
	m_iox_ret       =   0x00;
	m_iox_cmd       =   0xff;
	m_iox_leds      =   0x00;
	m_iox_coins     =   0x00;
}

void thedealr_state::machine_reset()
{
	iox_reset();
}

// 3400
uint8_t thedealr_state::iox_r()
{
	uint8_t ret = m_iox_ret;
	m_iox_status &= ~IOX_OUT_FULL;

	logerror("%s: IOX read %02X\n", machine().describe_context(), ret);
	return ret;
}
void thedealr_state::iox_w(uint8_t data)
{
	if (m_iox_status & IOX_WAITDATA)
	{
		m_iox_status &= ~IOX_WAITDATA;
		logerror("%s: IOX data %02X <- %02X\n", machine().describe_context(), m_iox_cmd, data);

		switch (m_iox_cmd)
		{
			case 0x20:  // leds
				m_iox_leds = data;
				m_leds[0] = BIT(data, 0);  // bet
				m_leds[1] = BIT(data, 1);  // deal
				m_leds[2] = BIT(data, 2);
				m_leds[3] = BIT(data, 3);
				m_leds[4] = BIT(data, 4);  // hold 1-5?
				m_leds[5] = BIT(data, 5);
				m_leds[6] = BIT(data, 6);
				m_leds[7] = BIT(data, 7);
				break;

			case 0x40:  // coin counters
				m_iox_coins = data;
				machine().bookkeeping().coin_counter_w(0, (~data) & 0x02); // coin1 or service coin
				machine().bookkeeping().coin_counter_w(1, (~data) & 0x04); // coupon
				machine().bookkeeping().coin_counter_w(2, (~data) & 0x08); // service coin
				machine().bookkeeping().coin_counter_w(3, (~data) & 0x10); // coin-out
				if ((~data) & 0xe1)
					logerror("%s: unknown bits written to command %02X: %02X\n", machine().describe_context(), m_iox_cmd, data);
				break;

			default:
				logerror("%s: data for unknown command written %02X = %02X\n", machine().describe_context(), m_iox_cmd, data);
				break;
		}

//      popmessage("LED: %02X COIN: %02X", m_iox_leds, m_iox_coins);
	}
	else
	{
		m_iox_cmd = data;
		logerror("%s: IOX command %02X\n", machine().describe_context(), m_iox_cmd);

		switch (m_iox_cmd)
		{
			case 0x01:  // inputs?
			{
				uint16_t buttons = m_iox_io->read();
				m_iox_ret = 0;
				for (int i = 0; i < 16; ++i)
				{
					if (buttons & (1<<i))
					{
						m_iox_ret = i + 1;
						break;
					}
				}
				m_iox_status |= IOX_OUT_FULL;
				break;
			}

//          case 0x04:  // ? at boot

			case 0x08:  // return iox version
				m_iox_ret = 0x54;
				m_iox_status |= IOX_OUT_FULL;
				break;

			case 0x20:  // leds
				m_iox_status |= IOX_WAITDATA;
				break;

			case 0x40:  // coin counters
				m_iox_status |= IOX_WAITDATA;
				break;

			case 0x80:  // store param?
				m_iox_status |= IOX_WAITDATA;
				break;

			case 0x81:  // store param?
				m_iox_status |= IOX_WAITDATA;
				break;

			case 0xff:  // reset
				iox_reset();
				break;

			default:
				logerror("%s: IOX unknown command %02X\n", machine().describe_context(), m_iox_cmd);
		}
	}
}

// 3401
uint8_t thedealr_state::iox_status_r()
{
	// bit 0 - Out buff full?
	// bit 1 - In  buff full?
	// bit 7 - Ready for more data?
	return m_iox_status;
}

/***************************************************************************

    Memory Maps - Main CPU

***************************************************************************/

uint8_t thedealr_state::irq_ack_r()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0;
}

void thedealr_state::unk_w(uint8_t data)
{
	// bit 1 - ? 1 during game
	// bit 2 - ? 0 during game
	// bit 3 - ? 1 during game
	// bit 7 - ? 0 during game
//  popmessage("UNK %02x", data);
}

void thedealr_state::thedealr_main(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");

	map(0x2000, 0x2000).ram(); // w ff at boot (after clearing commram)

	map(0x2400, 0x2400).r(FUNC(thedealr_state::irq_ack_r)); // r = irq ack.
	map(0x2400, 0x2400).w(FUNC(thedealr_state::unk_w));    // w = ?

	map(0x2800, 0x2800).portr("COINS").nopw();  // rw

	map(0x2801, 0x2801).portr("DSW4");
	map(0x2c00, 0x2c00).portr("DSW3");

	map(0x3400, 0x3400).rw(FUNC(thedealr_state::iox_r), FUNC(thedealr_state::iox_w));
	map(0x3401, 0x3401).r(FUNC(thedealr_state::iox_status_r));

	map(0x3000, 0x3000).ram(); // rw, comm in test mode
	map(0x3001, 0x3001).ram(); // rw, ""

	map(0x3800, 0x3bff).ram().share("commram");

	map(0x3c00, 0x3c00).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x3c01, 0x3c01).w("aysnd", FUNC(ay8910_device::data_w));

	map(0x8000, 0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

/***************************************************************************

    Memory Maps - Sub CPU

***************************************************************************/

void thedealr_state::thedealr_sub(address_map &map)
{
	// Work RAM
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram();

	// Sprites
	map(0x0800, 0x27ff).ram().rw(m_seta001, FUNC(seta001_device::spritecodelow_r8), FUNC(seta001_device::spritecodelow_w8));
	map(0x2800, 0x3fff).ram().rw(m_seta001, FUNC(seta001_device::spritecodehigh_r8), FUNC(seta001_device::spritecodehigh_w8));
	map(0x4000, 0x42ff).ram().rw(m_seta001, FUNC(seta001_device::spriteylow_r8), FUNC(seta001_device::spriteylow_w8));
	map(0x4300, 0x4303).w(m_seta001, FUNC(seta001_device::spritectrl_w8));
	map(0x4800, 0x4800).w(m_seta001, FUNC(seta001_device::spritebgflag_w8));   // enable / disable background transparency

	// Comm RAM
	map(0x5800, 0x5bff).ram().share("commram");

	// ROM
	map(0x6000, 0x7fff).rom().region("subcpu", 0x0000);
	map(0x8000, 0xffff).rom().region("subcpu", 0x8000);
}

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( thedealr )
	PORT_START("IOX")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD5   ) // HL5 (hold 5)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_GAMBLE_HALF   ) // 1/2 (half gamble)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW    ) PORT_NAME("Small") // SML (small)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SERVICE1      ) PORT_NAME("Reset") // RST (reset)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // PAY
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SERVICE2      ) // (unused?)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_BET    ) // BET (bet)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_CANCEL  ) // MET (cancel? keep pressed to show stats)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH   ) PORT_NAME("Big") // BIG (big)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP   ) // D.U (double up?)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL   ) PORT_NAME("Deal / Draw") // D.D (deal/draw, advance in test mode, trigger leds)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE   ) // T.S (take score?)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_POKER_HOLD4   ) // HL4 (hold 4)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_POKER_HOLD3   ) // HL3 (hold 3)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_POKER_HOLD2   ) // HL2 (hold 2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD1   ) // HL1 (hold 1)
	// DRP in test mode is an optional (via DSW) coin drop sensor?

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("MSN?") // !MSN
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_NAME("Attendant Clear?") // !ACL (reset jackpots, only if there are no credits)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // A.P (attendant payout? clears credits, port 0 = ef)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT          ) // TLT (tilt)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM       ) // HOV (hopper?)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) // CPN (coupon, port 3 = fb)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5) // CS2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5) // CS1 (coin1, port 3 = fd)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "SW1:1,2,3" ) PORT_DIPLOCATION("SW1:1,2,3") // table at EEFC
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x18, "SW1:4,5" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
/*
 Switches 7 & 8 control the payout as follows:

            Off/Off Off/On  On/Off  On/On   Notes
            ---------------------------------------------------------------------------------------------
Jackpot MB   5000    5000    5000    2000   Ryl Flush bonus at Max Bet
   Jackpot   2500    2500    2500    1000   Ryl Flush bonus at 5 coins + 500 (or 200) per coin up to Max Bet
   Mini JP   1500    1500    1000     500   Str Flush bonus at Max Bet
 Ryl Flush    500     500     500     200   x Bet
 Str Flush    150     150     100      50   x Bet
        4K     60      60      40      25   x Bet
Full House     10      10      10       8   x Bet
     Flush      7       6       7       6   x Bet
  Straight      5       5       5       5   x Bet
        3K      3       3       3       4   x Bet
  Two pair      2       2       2       2   x Bet
    Jacks+      1       1       1       1   x Bet (When enabled - DSW 3-2)

Return Rate 111.9%  110.7%  106.6%  105.8%  Jacks or Better
Return Rate  94.7%   92.9%   89.4%   87.8%  Two Pair

NOTE: Jackpot & Mini Jackpot values based on 10 Coin Max Bet. Values increase with higher Max Bet values.

Calculated returns based on 1 coin bet and paytable as shown above, Two Pair through Royal Flush without bonuses.
*/
	PORT_DIPNAME( 0xc0, 0xc0, "Payout Percentage" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "87.8%" )  PORT_CONDITION("DSW3", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x40, "89.4%" )  PORT_CONDITION("DSW3", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x80, "92.9%" )  PORT_CONDITION("DSW3", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0xc0, "94.7%" )  PORT_CONDITION("DSW3", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, "105.8%" ) PORT_CONDITION("DSW3", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, "106.6%" ) PORT_CONDITION("DSW3", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, "110.7%" ) PORT_CONDITION("DSW3", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, "111.9%" ) PORT_CONDITION("DSW3", 0x02, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x18, 0x18, "SW2:4,5" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xe0, 0xe0, "Max Bet" ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0xe0, "10" )
	PORT_DIPSETTING(    0xa0, "20" )
	PORT_DIPSETTING(    0x80, "20 Duplicate" )
	PORT_DIPSETTING(    0x40, "30" )
	PORT_DIPSETTING(    0x00, "30 Duplicate" )
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x20, "60 Duplicate" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPNAME( 0x02, 0x02, "Lowest Paid Hand" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "Two Pair" )
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
/*
Fever Mode:
  Overrides Jacks or Better
  3 of a Kind winning hand Jacks or higher enters Fever Mode
  You start with a pair of your 3 of a Kind cards & you draw 3 cards each hand.
    Jacks through Kings get 5 Fever Mode Draws
    Aces get 15 Fever Mode Draws
*/
	PORT_DIPNAME( 0x20, 0x20, "Fever Mode" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" ) /* Overrides Coinage - 1C / 1C */
	PORT_DIPNAME( 0x80, 0x80, "Coin In / Coin Out" ) PORT_DIPLOCATION("SW3:8") /* No credit - payout waiting for hopper??? */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:7" )   // X in service mode
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:8" )   // ""
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW4:2" )   // "Excess switch time" error
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW4:6" )
INPUT_PORTS_END

/***************************************************************************

    Graphics Layouts

***************************************************************************/

// The bitplanes are separated (but there are 2 per rom)
static const gfx_layout layout_planes_2roms =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0},
	{0,1,2,3,4,5,6,7, 128,129,130,131,132,133,134,135},
	{0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		16*16,17*16,18*16,19*16,20*16,21*16,22*16,23*16 },
	16*16*2
};

static GFXDECODE_START( gfx_thedealr )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms, 0, 32 )
GFXDECODE_END

/***************************************************************************

    Machine Drivers

***************************************************************************/

void thedealr_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_iox_status));
	save_item(NAME(m_iox_ret));
	save_item(NAME(m_iox_cmd));
	save_item(NAME(m_iox_leds));
	save_item(NAME(m_iox_coins));
}

/*
    It takes $19 IRQs to cycle through the IOX routines (read version/inputs, write outputs).
    With a single IRQ per frame this would translate to acknowledging inputs only every 0.4 seconds!
    Hence we generate more IRQs than that in a frame. Besides, with a single VBLANK IRQ,
    the two 65C02 fail to sync properly on boot with error $26 and/or $27 (commram[0]) !?
*/
TIMER_DEVICE_CALLBACK_MEMBER(thedealr_state::thedealr_interrupt)
{
	int scanline = param;

	if((scanline % 8) == 0)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

void thedealr_state::thedealr(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, XTAL(16'000'000)/8);   // 2 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &thedealr_state::thedealr_main);

	TIMER(config, "scantimer").configure_scanline(FUNC(thedealr_state::thedealr_interrupt), "screen", 0, 1);

	R65C02(config, m_subcpu, XTAL(16'000'000)/8);    // 2 MHz?
	m_subcpu->set_addrmap(AS_PROGRAM, &thedealr_state::thedealr_sub);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	SETA001_SPRITE(config, m_seta001, 16'000'000, m_palette, gfx_thedealr);
	m_seta001->set_bg_yoffsets(  0x11+1, -0x10 );   // + is up (down with flip)
	m_seta001->set_fg_yoffsets( -0x12+1, -0x01 );

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 384-1, 0+30, 256-1);
	screen.set_screen_update(FUNC(thedealr_state::screen_update));
	screen.screen_vblank().set(FUNC(thedealr_state::screen_vblank));
	screen.screen_vblank().append_inputline(m_subcpu, INPUT_LINE_NMI);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(thedealr_state::thedealr_palette), 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ym2149_device &aysnd(YM2149(config, "aysnd", XTAL(16'000'000)/8));   // 2 MHz?
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

/***************************************************************************

    ROMs Loading

***************************************************************************/

ROM_START( thedealr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "xb4_002", 0x00000, 0x08000, CRC(022fc8c4) SHA1(f29909ac22df9390f5bc1dec5f4dc5ae2c4d2e61) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "xb3_002", 0x00000, 0x10000, CRC(53a37fa4) SHA1(2adfea2dd08f298cda885bc72606d03f8af886a0) )

	// To do: hook up
	ROM_REGION( 0x0800, "iocpu", 0 )
	ROM_LOAD( "x0-009",  0x0000, 0x0800, CRC(e8b86d5a) SHA1(ad12e8f4411c30cd691792c6b0b3429db786d8b5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "xb0-002-w45.u42", 0x00000, 0x80000, CRC(41ec6a57) SHA1(d3f0508d5f4054fd2b0ee5227325a95fd1272aad) )
	ROM_LOAD( "xb0-001-w44.u41", 0x80000, 0x80000, CRC(bdaca555) SHA1(5ae1dc1514993fd804a101182735d5fb6815f720) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "xb0-u65.u65", 0x000, 0x200, CRC(5969a133) SHA1(f92f17949c1974d779d31741afd137f9217af3b9) )
	ROM_LOAD( "xb0-u68.u68", 0x200, 0x200, CRC(c0c54d43) SHA1(5ce352fb888c8e683014c73e6da00ec95f2ae572) )
ROM_END

GAME( 1988?, thedealr, 0, thedealr, thedealr, thedealr_state, empty_init, ROT0, "Visco Games", "The Dealer (Visco)", MACHINE_SUPPORTS_SAVE )
