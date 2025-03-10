// license:BSD-3-Clause
// copyright-holders: Jim Stolis, Roberto Fresca, Grull Osgo
/*******************************************************************************************************

  DRAW 80 POKER
  Sircoma / IGT

  Driver by Jim Stolis, Roberto Fresca & Grull Osgo.


  --- Technical Notes ---

  Name:    Draw 80 Poker
  Company: IGT - International Game Technology
  Year:    1982

  Hardware:

  CPU =  INTEL 8039       ; Intel MCS-48 family of 8-bit microcontrollers
  VIDEO = SYS 6545        ; CRTC M6845 compatible
  SND =  AY-3-8912        ; AY8910 compatible

  History:

  This is one of the first video machines produced by IGT.  Originally, the
  company was called SIRCOMA and was founded in 1979.  It became a public
  company in 1981 and changed its name to IGT.


  Coinage:

    DSW#3      OFF        ON         OFF       ON
    DSW#4      OFF        OFF        ON        ON

    Coin1        1         3          5        10
    A/S/H      4+4     12+12      20+20     40+40
    G         20+4     60+12     100+20    200+40


********************************************************************************************************

  Notes by game:
  -------------

  * Draw 80 Poker:

  Draw 80 Poker challenges even the most skilled players with a computerized dealer. Each hand
  offers players the choice to stand pat, discard, and draw from a reshuffled deck. Players can
  wager one to eight coins or skill points per hand.

  Draw 80 Poker where excitement, reliability, and the potential for earnings come together
  in a thrilling experience.

  Please note: The operation of these games and their features may be subject to state and local
  laws or regulations. This content is not intended to solicit the sale or operation of these
  games in jurisdictions where such activities are not legally permitted.


  Features:

  - Available in 13" color screen or 19" b/w screen.
  - Microprocessor based circuitry with back-up battery for memory.
  - High security cash box system (optional).
  - Multiply coin entry, single coin option.
  - Keyed reset for anti-cheat override.
  - Ticket or token dispenser (optional, check local law).
  - Credit register.
  - Cashbox door.
  - Dual coin entry.
  - Star-point button switches.


  * Wild 1 (Quick Change Kit):

  The IGT Drop In Amusement is a compact, versatile video unit designed for easy installation
  on bar tops, cocktail tables, or virtually any flat horizontal surface. It offers the exciting
  benefit of having all the Quick Change games in a single unit.

  Servicing and game conversions can be done without removing the Drop In Amusement from its location,
  making it incredibly convenient. Turn unused space into a profitable opportunity with this innovative
  solution.


********************************************************************************************************

  Updates [2025-02-24]

  - Added the DIP switch via AY8910.
  - Battery sensor through T1 line.
  - Rewrite the input system for players.
  - Improved coin inputs.
  - Added and documented the DIP switches fuctionalities.
  - Lamps support.
  - Mech Counters support.
  - Added hopper device.
  - Added default init for video memory.
  - Fixed a bug in read NVRAM decode.
  - Added AY-8910 callback for the DIP switch.
  - Added support for Wild 1 cocktail mode.
  - Added button-lamps clickable layout for both games.
  - System is playable with almost complete functionality.

  Updates [2025-02-27]

  - Fixed inputs polarity: No more credits triggered at reset.
  - Found the "Pair of Aces" DIP switch.
  - Fixed hopper DIP switch polarity.
  - Fixed hopper coin out signal.
  - Reverse-engineered the whole DIP switches bank.
  - Rewrote I/O handlers to simplify the if/then nested scheme toward switch statements.
  - Reworked coin inputs per game.
  - Promoted Draw 80 Poker (Minn) to working.
  - Promoted Wild 1 (Quick Change Kit) to working.
  

  TODO:

  - Support for the IGT ABC type coin acceptor.
  - Reverse and find the unknown port access via P1.5 and P1.2.
  - Find the coin lock line for documentation.
  - Find if there is a diverter or weight sensor line.
  - Reverse the behaviour of the port line access called "attract mode".


*******************************************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "drw80pkr.lh"


namespace {

class drw80pkr_state : public driver_device
{
public:
	drw80pkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_crtc(*this, "crtc"),
		m_aysnd(*this, "aysnd"),
		m_mainbank(*this, "mainbank"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U)

	{ }

	void init_drw80pkr();
	void drw80pkr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	tilemap_t *m_bg_tilemap;
	uint8_t m_t0;
	uint8_t m_t1;
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_prog;
	uint8_t m_bus;
	uint8_t m_attract_mode;
	uint8_t m_active_bank;
	uint8_t m_pkr_io_ram[0x100];
	uint16_t m_video_ram[0x0400] = {};
	uint8_t m_color_ram[0x0400] = {};

	required_device<i8039_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<mc6845_device> m_crtc;
	required_device<ay8912_device> m_aysnd;
	required_memory_bank m_mainbank;
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<8> m_lamps;

	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	void prog_w(int state);
	void bus_w(uint8_t data);
	void io_w(offs_t offset, uint8_t data);
	int t0_r();
	int t1_r();
	uint8_t p1_r();
	uint8_t p2_r();
	uint8_t bus_r();
	uint8_t io_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void drw80pkr_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

};

#define DATA_NVRAM_SIZE     0x100


/***********************************
*          Machine Start           *
***********************************/

void drw80pkr_state::machine_start()
{
	subdevice<nvram_device>("nvram")->set_base(m_pkr_io_ram, sizeof(m_pkr_io_ram));

	m_active_bank = 0;
	m_lamps.resolve();
	m_t1 = 1;  // battery level sensor (1 = battery ok)
}


/***********************************
*          Write Handlers          *
***********************************/

void drw80pkr_state::p1_w(uint8_t data)
{
	m_p1 = data;
}

void drw80pkr_state::p2_w(uint8_t data)
{
	m_p2 = data;
}

void drw80pkr_state::prog_w(int state)
{
	m_prog = state;

	// Bankswitch Program Memory
	if (m_prog == 0x01)
	{
		m_active_bank = m_active_bank ^ 0x01;
		m_mainbank->set_entry(m_active_bank);
	}
}

void drw80pkr_state::bus_w(uint8_t data)
{
	m_bus = data;
}

void drw80pkr_state::io_w(offs_t offset, uint8_t data)
{
	uint16_t n_offs;

	switch (m_p2)
	{
		case 0x3f:
		{
			n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			m_video_ram[n_offs] = data;  // low address
			m_bg_tilemap->mark_tile_dirty(n_offs);
			break;
		}
		case 0x7f:
		{
			n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			m_color_ram[n_offs] = data & 0x0f;  // color palette
			m_video_ram[n_offs] += ((data & 0xf0) << 4 );  // high address
			m_bg_tilemap->mark_tile_dirty(n_offs);
			break;
		}
		case 0xc7: m_crtc->address_w(data); break;
		case 0xd7: m_crtc->register_w(data); break;
		case 0xfb: m_pkr_io_ram[offset] = data; break;
		case 0xff:
		{
			switch (m_p1)
			{
				case 0xf7:
				{
					machine().bookkeeping().coin_counter_w(0, data & 0x01);  // coin in counter
					machine().bookkeeping().coin_counter_w(1, data & 0x02);  // credits paid
					machine().bookkeeping().coin_counter_w(2, data & 0x04);  // credits bet
					machine().bookkeeping().coin_counter_w(3, data & 0x08);  // coin out counter
					m_hopper->motor_w(BIT(data, 4) && BIT(data, 5));
					m_lamps[7] = BIT(data, 4) && BIT(data, 5);               // for testing purpose
					break;
				}
				case 0xef:
				{
					m_lamps[0] = BIT(data, 0);  // bet lamp
					m_lamps[1] = BIT(data, 1);  // start (deal/draw)
					m_lamps[2] = BIT(data, 2);  // hold
					m_lamps[3] = BIT(data, 3);  // hold
					m_lamps[4] = BIT(data, 4);  // hold
					m_lamps[5] = BIT(data, 5);  // hold
					m_lamps[6] = BIT(data, 6);  // hold
					break;
				}
				case 0xdf: m_attract_mode = data; break;  // unknown port access via P1.5
				case 0xfc: m_aysnd->address_w(data); break;
				case 0xfe: m_aysnd->data_w(data); break;
				case 0xfb: break;  // unknown port access via P1.2
				case 0xdb: break;  // unknown port access via P1.5 and P1.2
				//default: logerror("%s - Unknown I8039 - IO_W with Port 1:%02x access - data:%02x\n", machine().describe_context(), m_p1, data);
			} break;
		}
		//default: logerror("%s - Unknown I8039 - IO_W with Port 2:%02x access - data:%02x\n", machine().describe_context(), m_p2, data);
	}
}


/***********************************
*          Read Handlers           *
***********************************/

int drw80pkr_state::t0_r()
{
	return m_t0;
}

int drw80pkr_state::t1_r()
{
	return m_t1;
}

uint8_t drw80pkr_state::p1_r()
{
	return m_p1;
}

uint8_t drw80pkr_state::p2_r()
{
	return m_p2;
}

uint8_t drw80pkr_state::bus_r()
{
	return m_bus;
}

uint8_t drw80pkr_state::io_r(offs_t offset)
{
	uint8_t ret = 0;

	switch (m_p2)
	{
		case 0x3b: break;  // dummy read done by "orl  p2,#$FF" instruction.
		case 0xf7: break;  // dummy read done by "orl  p2,#$FF" instruction.
		case 0x7b:         // some program bug, but ignored by real hardware.
		case 0xfb: ret = m_pkr_io_ram[offset]; break;
		case 0xff:
		{
			switch (m_p1)
			{
				case 0xdb:
				case 0x5f:
				case 0x9f: break;  // dummy reads done by "orl  p1,#$FF" instruction.
				case 0xfe: ret = m_aysnd->data_r(); break;
				case 0xef:
				{
					switch (ioport("IN1")->read())
					{
						// Encoded bits 0, 1, 2
						case 0x00: ret = 0xc0; break;  // A Default port value                  11-000-000
						case 0x01: ret = 0xc4; break;  //   Keyout                              11-000-100
						case 0x02: ret = 0xc6; break;  // A KeyIn A (Coinage x 1 ) No timeout   11-000-110
						// Direct bits 3, 4, 5
						case 0x04: ret = 0xc8; break;  // 5 Coin (Timeout) (Coin_A Sequencer)   11-001-000 - Coin option only for drw80wld
						case 0x08: ret = 0xd0; break;  // 6 x              (Coin_B Sequencer)   11-010-000
						case 0x10: ret = 0xe0; break;  // 7 x              (Coin_C Sequencer)   11-100-000
						// Direct Bits 6, 7
						case 0x20: ret = 0x80; break;  // S Key In B (Coinage x 4)  No Timeout  10-000-000
						case 0x40: ret = 0x40; break;  // D Key In C (Coinage x 20) No Timeout  01-000-000
					}
					break;
				}
				case 0xf7:
				{
					switch (ioport("IN2")->read())
					{
						case 0x0000: ret = 0x80; break;
						case 0x0001: ret = 0x81; break;  // HOLD 4 P1
						case 0x0002: ret = 0x82; break;  // HOLD 3 P1
						case 0x0004: ret = 0x83; break;  // HOLD 2 P1
						case 0x0008: ret = 0x84; break;  // HOLD 1 P1
						case 0x0010: ret = 0x85; break;  // START  P1
						case 0x0020: ret = 0x86; break;  // BET    P1
						case 0x0040: ret = 0x87; break;  // HOLD 5 P1
						case 0x0080: ret = 0x88; break;  // HOLD 4 P2
						case 0x0100: ret = 0x90; break;  // HOLD 3 P2
						case 0x0200: ret = 0x98; break;  // HOLD 2 P2
						case 0x0400: ret = 0xa0; break;  // HOLD 1 P2
						case 0x0800: ret = 0xa8; break;  // START  P2
						case 0x1000: ret = 0xb0; break;  // BET    P2
						case 0x2000: ret = 0xb8; break;  // HOLD   P2
						case 0x4000: ret = 0xc0; break;  // Hopper coin out
						case 0x8000: ret = 0x00; break;  // Books/Door
					} break;
				}
				default:
				{
					//logerror("%s - Unknown I8039 - I/O with Port 1:%02x access\n", machine().describe_context(), m_p1);
					ret = 0;
				}
			}
		} break;
		default:
		{
			//logerror("%s - Unknown I8039 - I/O with Port 2:%02x access\n", machine().describe_context(), m_p2);
			ret = 0;
		}
	}
	return ret;
}


/***********************************
*          Video Hardware          *
***********************************/

TILE_GET_INFO_MEMBER(drw80pkr_state::get_bg_tile_info)
{
	int color = m_color_ram[tile_index];
	int code = m_video_ram[tile_index];

	tileinfo.set(0, code, color, 0);
}

void drw80pkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drw80pkr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 24, 27);
}

uint32_t drw80pkr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void drw80pkr_state::drw80pkr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int j = 0; j < palette.entries(); j++)
	{
		int const i = BIT(color_prom[j], 3);

		// red component
		int const tr = 0xf0 - (0xf0 * BIT(color_prom[j], 0));
		int const r = tr - (i * (tr / 5));

		// green component
		int const tg = 0xf0 - (0xf0 * BIT(color_prom[j], 1));
		int const g = tg - (i * (tg / 5));

		// blue component
		int const tb = 0xf0 - (0xf0 * BIT(color_prom[j], 2));
		int const b = tb - (i * (tb / 5));

		palette.set_pen_color(j, rgb_t(r, g, b));
	}
}


/*****************************************
*            Graphics Layouts            *
*****************************************/

static const gfx_layout charlayout =
{
	8,8,            // 8x8 characters
	RGN_FRAC(1,2),  // 512 characters
	2,              // 2 bitplanes
	{ 0, RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


/*****************************************
*      Graphics Decode Information       *
*****************************************/

static GFXDECODE_START( gfx_drw80pkr )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 16 )
GFXDECODE_END


/*****************************************
*              Driver Init               *
*****************************************/

void drw80pkr_state::init_drw80pkr()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
}


/*****************************************
*         Memory Map Information         *
*****************************************/

void drw80pkr_state::map(address_map &map)
{
	map(0x0000, 0x0fff).bankr("mainbank");
}

void drw80pkr_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(drw80pkr_state::io_r), FUNC(drw80pkr_state::io_w));
}


/*****************************************
*              Input Ports               *
*****************************************/

static INPUT_PORTS_START( drw80pkr )

	PORT_START("IN1")  //$EF
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("KeyIn x01") // KeyIn A (Coinage x 1 )  No timeout
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("KeyIn x04") // Key In B (Coinage x 4)  No Timeout
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("KeyIn x20") // Key In C (Coinage x 20) No Timeout

	PORT_START("IN2")  //$F7
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_IMPULSE(2) // Double -> YES / HIGH
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL ) PORT_IMPULSE(2) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )  PORT_IMPULSE(2) PORT_NAME("Bet")  // switch the control for cocktail mode.
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_IMPULSE(2) // Double -> NO / LOW

	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode hold 4
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode hold 3
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode hold 2
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode hold 1
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode deal/draw
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode bet
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )  // unsupported cocktail mode hold 5
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Pair of Aces" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Joker Enable" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Enabled" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPNAME( 0x18, 0x00, "Payout Type" )         PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "Cash" )
	PORT_DIPSETTING(    0x08, "Credits" )
	PORT_DIPSETTING(    0x10, "Credits" )
	PORT_DIPSETTING(    0x18, "Credits" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Acceptor Type" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "Type ABC" )
	PORT_DIPSETTING(    0x00, "Single Pulse" )
INPUT_PORTS_END


static INPUT_PORTS_START( drw80wld )
	PORT_INCLUDE( drw80pkr )

	PORT_MODIFY("IN1")  //$EF
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("KeyIn x01") PORT_CODE(KEYCODE_A) // KeyIn A (Coinage x 1 )  No timeout
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )  // coin sequencer A  Or Coin Switch (DSW_1 Selector)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )	 // coin sequencer B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 )	 // coin sequencer C
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("KeyIn x04") PORT_CODE(KEYCODE_S) // Key In B (Coinage x 4)  No Timeout
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("KeyIn x20") PORT_CODE(KEYCODE_D) // Key In C (Coinage x 20) No Timeout

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Hold 4 (cocktail mode)") PORT_IMPULSE(2)  // cocktail mode hold 4
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Hold 3 (cocktail mode)") PORT_IMPULSE(2)  // cocktail mode hold 3
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Hold 2 (cocktail mode)") PORT_IMPULSE(2)  // cocktail mode hold 2
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Hold 1 (cocktail mode)") PORT_IMPULSE(2)  // cocktail mode hold 1
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("Deal (cocktail mode)")   PORT_IMPULSE(2)  // cocktail mode deal/draw
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Bet")                    PORT_IMPULSE(2)  // cocktail mode bet (switch the control for cocktail mode)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Hold 5 (cocktail mode)") PORT_IMPULSE(2)  // cocktail mode hold 5
INPUT_PORTS_END


/*****************************************
*             Machine Driver             *
*****************************************/

void drw80pkr_state::drw80pkr(machine_config &config)
{
	// basic machine hardware
	I8039(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &drw80pkr_state::map);
	m_maincpu->set_addrmap(AS_IO, &drw80pkr_state::io_map);
	m_maincpu->t0_in_cb().set(FUNC(drw80pkr_state::t0_r));
	m_maincpu->t1_in_cb().set(FUNC(drw80pkr_state::t1_r));
	m_maincpu->p1_in_cb().set(FUNC(drw80pkr_state::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(drw80pkr_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(drw80pkr_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(drw80pkr_state::p2_w));
	m_maincpu->prog_out_cb().set(FUNC(drw80pkr_state::prog_w));
	m_maincpu->bus_in_cb().set(FUNC(drw80pkr_state::bus_r));
	m_maincpu->bus_out_cb().set(FUNC(drw80pkr_state::bus_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8_MHz_XTAL / 2, 256, 0, 192, 257, 0, 216);  // 4 MHz?
	screen.set_screen_update(FUNC(drw80pkr_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_drw80pkr);

	PALETTE(config, "palette", FUNC(drw80pkr_state::drw80pkr_palette), 16 * 16);

	MC6845(config, m_crtc, 8_MHz_XTAL / 16);  // 0.5 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8912(config, m_aysnd, 20000000/12).add_route(ALL_OUTPUTS, "mono", 0.75);
	m_aysnd->port_a_read_callback().set_ioport("DSW1");

	HOPPER(config, m_hopper, attotime::from_msec(150));
}


/*****************************************
*                Rom Load                *
*****************************************/

ROM_START( drw80pkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(0f3e97d2) SHA1(aa9e4015246284f32435d7320de667e075412e5b) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(5a6ad467) SHA1(0128bd70b65244a0f68031d5f451bf115eeb7609) )

	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",   0x0000, 0x1000, CRC(97f5eb92) SHA1(f6c7bb42ccef8a78e8d56104ad942ae5b8e5b0df) )
	ROM_LOAD( "cg1-a.u76",   0x1000, 0x1000, CRC(2a3a750d) SHA1(db6183d11b2865b011c3748dc472cf5858dde78f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

ROM_START( drw80wld )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(73223555) SHA1(229999ec00a1353f0d4928c65c8975079060c5af) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(f8158f2b) SHA1(da3b30cfd49cd0e8a48d78fd3f82b2b4ab33670c) )

	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",   0x0000, 0x1000, CRC(0eefe598) SHA1(ed10aac345b10e35fb15babdd3ac30ebe2b8fc0f) )
	ROM_LOAD( "cg1-a.u76",   0x1000, 0x1000, CRC(57d39a16) SHA1(c3d826c2d427bcaabed6a71c1c34d5411afdace8) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

} // Anonymous namespace


/*****************************************
*              Game Drivers              *
*****************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT           ROT     COMPANY                                FULLNAME                    FLAGS     LAYOUT
GAMEL( 1983, drw80pkr, 0,      drw80pkr, drw80pkr, drw80pkr_state, init_drw80pkr, ROT0,  "IGT - International Game Technology", "Draw 80 Poker (Minn)",      0,        layout_drw80pkr)
GAMEL( 1982, drw80wld, 0,      drw80pkr, drw80wld, drw80pkr_state, init_drw80pkr, ROT0,  "IGT - International Game Technology", "Wild 1 (Quick Change Kit)", 0,        layout_drw80pkr)
