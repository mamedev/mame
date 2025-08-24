// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli,hap,Roberto Fresca
/**********************************************************

  MEYCO 8080 BASED GAMBLING HARDWARE
  Meyco Games, Inc. [Sunnyvale, CA, USA]

  Driver by

  Tomasz Slanina
  Pierpaolo Prazzoli
  Hap
  Roberto Fresca.


  There are two versions: Black & White and Color.

  Wild Arrow was originally developed for the same 8080
  black & white motherboard as the blackjack game. It
  was programmed in machine code by hand using an EPROM
  blaster. Later, color hardware was added to the mother-
  board and an Intel MDS system was used for assembly
  language programming. The third design iteration was
  a complete hardware redesign using the 8088 and a mo-
  dular software system and micro-operating system
  written in 'C' and assembler.

***********************************************************

  Meyco Games was first called Gametronics (1977-1979),
  reorganized as Meyco Games until around 1985, then or-
  ganized as Meyer-Gillmann Industries until about 1989.

  Games: Joker's Wild, Casino Black Jack, Montana Draw,
         Super Stud, Wild Arrow.

  Hardware/Soft Design:  Darrell H. Smith.
  Concept/Game Design:   Robert Meyer & Julia Gillmann.

***********************************************************

  Wild Arrow (c) 1982 Meyco Games
  CPU: 8080A
  RAM: 411A (x48)
  XTal: 20.0

***********************************************************

  To initialize battery RAM, go into Meter Read mode (F1 -> 9),
  and then press the Meter Read + Reset buttons (9 + 0).

  If a game is not turned off properly, eg. exiting MAME
  in mid-game, it may run faulty on the next boot.
  Enable the Night Switch to prevent this.

***********************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"

#include "wldarrow.lh"
#include "mdrawpkr.lh"
#include "meybjack.lh"


namespace {

class meyc8080_state : public driver_device
{
public:
	meyc8080_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram_0(*this, "vram0")
		, m_videoram_1(*this, "vram1")
		, m_videoram_2(*this, "vram2")
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void meyc8080(machine_config &config);

private:
	void lights_1_w(uint8_t data);
	void lights_2_w(uint8_t data);
	void counters_w(uint8_t data);
	void meyc8080_dac_1_w(uint8_t data);
	void meyc8080_dac_2_w(uint8_t data);
	void meyc8080_dac_3_w(uint8_t data);
	void meyc8080_dac_4_w(uint8_t data);
	uint32_t screen_update_meyc8080(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void meyc8080_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_lamps.resolve(); }

	required_shared_ptr<uint8_t> m_videoram_0;
	required_shared_ptr<uint8_t> m_videoram_1;
	required_shared_ptr<uint8_t> m_videoram_2;
	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	output_finder<11> m_lamps;
};


/*************************************
 *
 *  Video system
 *
 *************************************/

uint32_t meyc8080_state::screen_update_meyc8080(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram_0.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data0 = m_videoram_0[offs];
		uint8_t data1 = m_videoram_1[offs];
		uint8_t data2 = m_videoram_2[offs];

		/* weird equations, but it matches every flyer screenshot -
		   perhaphs they used a look-up PROM? */
		uint8_t data_r = data0;
		uint8_t data_g = (data2 & ~data0) | (data2 & data1) | (~data2 & ~data1 & data0);
		uint8_t data_b = data0 ^ data1;

		for (int i = 0; i < 8; i++)
		{
			bitmap.pix(y, x) = rgb_t(pal1bit(data_r >> 7), pal1bit(data_g >> 7), pal1bit(data_b >> 7));

			data_r <<= 1;
			data_g <<= 1;
			data_b <<= 1;

			x++;
		}
	}

	return 0;
}



/*************************************
 *
 *  Lights and coin counter
 *
 *************************************/

void meyc8080_state::lights_1_w(uint8_t data)
{
/* Wild Arrow lamps

  - bits -
  7654 3210
  ---- ---x   Stop 3 lamp.
  ---- --x-   Stop 2 lamp.
  ---- -x--   Stop 1 lamp.
  ---- x---   Start lamp.
  ---x ----   Bet lamp.
  xxx- ----   Seems unused...


  Draw Poker lamps

  - bits -
  7654 3210
  ---- ---x   Unknown (light with deal when holds). Should be Cancel.
  ---- --x-   Unknown (light with deal when holds). Should be Stand (Hold All), seeing the Black Jack behaviour.
  ---- -x--   Bet lamp.
  ---- x---   Deal lamp.
  xxxx ----   Seems unused...


  Unk Black Jack lamps

  - bits -
  7654 3210
  ---- ---x   Split lamp.
  ---- --x-   Stand lamp.
  ---- -x--   Bet lamp.
  ---- x---   Hit lamp.
  xxxx ----   Seems unused...

*/
	m_lamps[0] = BIT(data, 0);  /* Lamp 0 */
	m_lamps[1] = BIT(data, 1);  /* Lamp 1 */
	m_lamps[2] = BIT(data, 2);  /* Lamp 2 */
	m_lamps[3] = BIT(data, 3);  /* Lamp 3 */
	m_lamps[4] = BIT(data, 4);  /* Lamp 4 */

	logerror("lights 1: %02x\n", data);
}


void meyc8080_state::lights_2_w(uint8_t data)
{
/* Wild Arrow unknown pulse...

  - bits -
  7654 3210
  --x- ----   Unk pulse...
  xx-x xxxx   Seems unused...

  Beating constantly except when a game is playing
  Maybe a coin lock?


  Draw Poker lamps

  - bits -
  7654 3210
  ---- ---x   Hold 5 lamp (inverted).
  ---- --x-   Hold 4 lamp (inverted).
  ---- -x--   Hold 3 lamp (inverted).
  ---- x---   Hold 2 lamp (inverted).
  ---x ----   Hold 1 lamp (inverted).
  xxx- ----   Seems unused...


  Unk Black Jack lamps

  - bits -
  7654 3210
  ---- ---x   Unknown.
  ---- --x-   Surrender lamp.
  ---- -x--   Unknown.
  ---- x---   Bonus lamp.
  ---x ----   Double-Down lamp.
  xxx- ----   Unknown.

*/
	m_lamps[5] = BIT(data, 0);  /* Lamp 5 */
	m_lamps[6] = BIT(data, 1);  /* Lamp 6 */
	m_lamps[7] = BIT(data, 2);  /* Lamp 7 */
	m_lamps[8] = BIT(data, 3);  /* Lamp 8 */
	m_lamps[9] = BIT(data, 4);  /* Lamp 9 */

	m_lamps[10] = BIT(data, 5); /* Lamp 10 (Game-Over) */

	logerror("lights 2: %02x\n", data);
}


void meyc8080_state::counters_w(uint8_t data)
{
/* Wild Arrow & Draw Poker counters

  - bits -
  7654 3210
  ---- ---x   Coin counter.
  ---- --x-   Payout pulse.
  ---- -x--   Bet pulse.
  ---- x---   Manual Keyout pulse (only mdrawpkra)
  xxxx ----   Seems unused...

*/
	machine().bookkeeping().coin_counter_w(0, ~data & 0x01); /* Coin1 */
	machine().bookkeeping().coin_counter_w(1, ~data & 0x04); /* Bets */
	machine().bookkeeping().coin_counter_w(2, ~data & 0x02); /* Payout */

	/* Only Draw Poker (2-11) (mdrawpkra) */
	machine().bookkeeping().coin_counter_w(3, ~data & 0x08); /* Manual Keyout */

	logerror("counters: %02x\n", ~data);
}



/*************************************
 *
 *  Audio system - this is a guess
 *
 *  The software repeatedly writes to
 *  either f000/f0ff or f00f/f0f0,
 *  so I suspect that A3/A4 have
 *  meaning.
 *
 *************************************/

void meyc8080_state::meyc8080_dac_1_w(uint8_t data)
{
	m_dac->write(0);
}


void meyc8080_state::meyc8080_dac_2_w(uint8_t data)
{
	m_dac->write(1);
}


void meyc8080_state::meyc8080_dac_3_w(uint8_t data)
{
	m_dac->write(2);
}


void meyc8080_state::meyc8080_dac_4_w(uint8_t data)
{
	m_dac->write(3);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void meyc8080_state::meyc8080_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x3800).portr("IN0");
	map(0x4000, 0x5fff).ram().share("vram0");
	map(0x6000, 0x7fff).ram().share("vram1");
	map(0x8000, 0x9fff).ram().share("vram2");
//  map(0xa000, 0xa0ff).ram();     // unknown... filled with 00's at boot time or when entering the service mode.
	map(0xcd00, 0xcdff).ram().share("nvram");
	map(0xf000, 0xf000).portr("BSW").w(FUNC(meyc8080_state::meyc8080_dac_1_w));
	map(0xf004, 0xf004).portr("IN1").w(FUNC(meyc8080_state::lights_1_w));
	map(0xf006, 0xf006).portr("IN2").w(FUNC(meyc8080_state::lights_2_w));
	map(0xf008, 0xf008).w(FUNC(meyc8080_state::counters_w));
	map(0xf00f, 0xf00f).w(FUNC(meyc8080_state::meyc8080_dac_2_w));
	map(0xf0f0, 0xf0f0).w(FUNC(meyc8080_state::meyc8080_dac_3_w));
	map(0xf0ff, 0xf0ff).w(FUNC(meyc8080_state::meyc8080_dac_4_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( wldarrow )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x04, 0x00, "Monitor" )
	PORT_DIPSETTING(    0x00, "Color" )
	PORT_DIPSETTING(    0x04, "B/W" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )      PORT_DIPLOCATION("BSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )      PORT_DIPLOCATION("BSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )      PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )      PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("BSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )      PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )      PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mdrawpkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )      PORT_DIPLOCATION("BSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )      PORT_DIPLOCATION("BSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )      PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )      PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bit Switch 5" )      PORT_DIPLOCATION("BSW:5") // no coinage sw
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )      PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )      PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )      PORT_CODE(KEYCODE_S) PORT_NAME("Stand (Hold All)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mdrawpkra )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )      PORT_DIPLOCATION("BSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )      PORT_DIPLOCATION("BSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )      PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )      PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("BSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )      PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )      PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )      PORT_CODE(KEYCODE_S) PORT_NAME("Stand (Hold All)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Manual Keyout")
INPUT_PORTS_END


static INPUT_PORTS_START( casbjack )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )      PORT_DIPLOCATION("BSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )      PORT_DIPLOCATION("BSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )      PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )      PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("BSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )      PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )      PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Split")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stand")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Hit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_A) PORT_NAME("Insurance")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_S) PORT_NAME("Surrender")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_D) PORT_NAME("Bonus")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Double Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void meyc8080_state::meyc8080(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, XTAL(20'000'000) / 10); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &meyc8080_state::meyc8080_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 4*8, 32*8-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(meyc8080_state::screen_update_meyc8080));

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.66); // unknown DAC
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( wldarrow )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "a1-v48.8k", 0x0000, 0x0800, CRC(05dd8056) SHA1(556ca28d090cbf1855618ba40fc631523bdfadd5) )
	ROM_LOAD( "a2-v48.7k", 0x0800, 0x0800, CRC(37df3acf) SHA1(a7f7f54af533dd8231bb20c526c053dd99e74863) )
	ROM_LOAD( "a3-v48.6k", 0x1000, 0x0800, CRC(1295cee2) SHA1(61b260eb907ee4bbf1460277d09e3205c1f6d8a0) )
	ROM_LOAD( "a4-v48.5k", 0x1800, 0x0800, CRC(5562614e) SHA1(7cb04d76e987944d385d40515396fc27ba00ae83) )
ROM_END


/*

  Meyco Games, 1981

  Dumper notes:

  - Board is approximately 16.5"x14.25", with no daughterboards or satellite add-on boards.
  - Board has 8 pin connector and 50 pin connector with no keying pins (see diagram)
  - Board has an 20.000MHz crystal @ C4 (see diagram below).
  - No dip switchs or sound chips on this board.
  - Board has 32 uPD411 RAM chips.
  - The 40 pin CPU is missing, Guru feels it will be an 8080 CPU.

  - Dump consists of:
    6x 2516 @ K3, K4, K5, K6, K7, K8.

     K      J     H       G      F             E      D      C       B       A
    |------------------------------------------------------------------------------|
   1|                                                                              |
   2|                                                                              |
   3|ROM.3k                                    DS0026CN                            |
   4|ROM.4k                                           20.000                      *|
   5|ROM.5k                      N8T26AN                                           |
   6|ROM.6k               C      898-1-R 1.5K  N8T26AN                            &|
   7|ROM.7k               P      N8T26AN                                          &|
   8|ROM.8k               U      N8T26AN                                          &|
   9|                                                                             &|
  10|                                                                              |
  11|                                                                              |
  12|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
  13|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
  14|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
  15|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
    |------------------------------------------------------------------------------|

  * = 8 pin connector
  & = 50 pin connector (meant for edge connector or to another pcb?)

*/

ROM_START( mdrawpkr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "tms2516.k8", 0x0000, 0x0800, CRC(2e5fc31e) SHA1(5ea01298051bc51250f67305ac8a65b0b94c120f) )
	ROM_LOAD( "tms2516.k7", 0x0800, 0x0800, CRC(baaf874e) SHA1(b7bb476ef873102979ad3252d19a26ee3a31d933) )
	ROM_LOAD( "tms2516.k6", 0x1000, 0x0800, CRC(a0e13c41) SHA1(17f78f91dae64c39f1a39a0b99a081af1d3bed47) )
	ROM_LOAD( "tms2516.k5", 0x1800, 0x0800, CRC(530a48fd) SHA1(e539962d19d884794ece2e426423f6b33d54058d) )
	ROM_LOAD( "tms2516.k4", 0x2000, 0x0800, CRC(bb1bd38a) SHA1(90256991eb1d030dd72e7e6f8d1a7cce22340b42) )
	ROM_LOAD( "tms2516.k3", 0x2800, 0x0800, CRC(30904dc8) SHA1(c82276aa0eb8f48d136ad8c15dd309c9b880c294) )
ROM_END


/*

  Board silkscreened:
  COPYRIGHT 1981 MEYCO GAMES
  MEYCO GAMES,INC.

  Lots of empty sockets...

  .k3  2516  handwritten sticker  JK 6 - 02-10
  .k4  2516  handwritten sticker  JK 5 - 02-10
  .k5  2516  handwritten sticker  JK 4 - 02-10
  .k6  2516  handwritten sticker  JK 3 - 02-11
  .k7  2516  handwritten sticker  JK 2 - 02-10
  .k8  2516  handwritten sticker  JK 1 - 02-11

  eproms were unmarked,
  but a handwritten sticket said they were 2516 eproms

  8080 cpu
  upd411d x24
  upd411a x24
  20Mhz crystal

  Etched in copper on Daughter board:
  MEYCO GAME
  PSI/O RAM
  102480

  5101 x2
  batteries x4

*/

ROM_START( mdrawpkra )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "jk1_02-11.k8", 0x0000, 0x0800, CRC(b4f6994d) SHA1(46fb2784b7b333f6755522fd27741a2fc4a4bc99) )
	ROM_LOAD( "jk2_02-10.k7", 0x0800, 0x0800, CRC(8830365d) SHA1(044bc92880b78fa2a6ed5e133b484ac7d34c455a) )
	ROM_LOAD( "jk3_02-11.k6", 0x1000, 0x0800, CRC(e1d5d38d) SHA1(523029185e2edc0351fa128d6494a5002cb2d7e7) )
	ROM_LOAD( "jk4_02-10.k5", 0x1800, 0x0800, CRC(27c8dbcc) SHA1(996732b16c46460400957b3ed7bc36f537258dd7) )
	ROM_LOAD( "jk5_02-10.k4", 0x2000, 0x0800, CRC(e3f18769) SHA1(7c98ca3f8b423200eb51ebe432591a98394ef952) )
	ROM_LOAD( "jk6_02-10.k3", 0x2800, 0x0800, CRC(72aee07f) SHA1(a6d6086f3a6181d5111d05ae779c3f7b363c7f14) )
ROM_END

ROM_START( casbjack )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "k8", 0x0000, 0x0800, CRC(c6d8a2f7) SHA1(1fcd3ad72a29f832ffaf37edcb74e84e21909496) )
	ROM_LOAD( "k7", 0x0800, 0x0800, CRC(6e30e5ae) SHA1(980096adefce4d4d97607b22f56f3acf246986ed) )
	ROM_LOAD( "k6", 0x1000, 0x0800, CRC(7e2cb0e1) SHA1(3ea8f3c051ba730a39404b718d93bcdd762834c3) )
	ROM_LOAD( "k5", 0x1800, 0x0800, CRC(536c83f9) SHA1(0dc9171c14f96bde68dc3b175abe31fa25753b48) )
	ROM_LOAD( "k4", 0x2000, 0x0800, CRC(178b0f95) SHA1(cea5922f5294958c59cacd42e30399fc329426d9) )
	ROM_LOAD( "k3", 0x2800, 0x0800, CRC(9cd6b843) SHA1(fb9c5c5ba96ebb75dc42e7c891d6da2a8a1ea6c1) )
	ROM_LOAD( "k2", 0x3000, 0x0800, CRC(5f82eafa) SHA1(4f5a4dc773ceae9a69ec532166047867db4ddadf) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT      CLASS           INIT        ROT   COMPANY              FULLNAME                                     FLAGS                                            LAYOUT
GAMEL(1982, wldarrow,  0,        meyc8080, wldarrow,  meyc8080_state, empty_init, ROT0, "Meyco Games, Inc.", "Wild Arrow (color, Standard V4.8)",         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_wldarrow ) // B&W version not dumped yet
GAMEL(1984, mdrawpkr,  0,        meyc8080, mdrawpkr,  meyc8080_state, empty_init, ROT0, "Meyco Games, Inc.", "Draw Poker - Joker's Wild (Standard)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_mdrawpkr ) // year not shown, but it is in mdrawpkra
GAMEL(1984, mdrawpkra, mdrawpkr, meyc8080, mdrawpkra, meyc8080_state, empty_init, ROT0, "Meyco Games, Inc.", "Draw Poker - Joker's Wild (02-11)",         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_mdrawpkr )
GAMEL(1983, casbjack,  0,        meyc8080, casbjack,  meyc8080_state, empty_init, ROT0, "Meyco Games, Inc.", "Casino Black Jack (color, Standard 00-05)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_meybjack ) // B&W version not dumped yet
