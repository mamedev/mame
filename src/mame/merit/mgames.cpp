// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, David Haywood
/*********************************************************************************

  Match Games.
  CRT 100 + CRT 810 boards system.
  Merit Industries, 1981.

  Driver by Angelo Salese, David Haywood & Roberto Fresca.

  Maybe the first Merit videopoker.
  The system has 4 different themed skill games, selectable through DIP switches:

  * "The Frog Pond"
  * "Pharaohs"
  * "Wild Bulls"
  * "The White Knight"

**********************************************************************************

  From the Flyer...

  FOR AMUSEMENT ONLY.

  The ingenious Match Games offers something for everyone. Four captivating themes
  with brilliant graphics, challenging play action, an enticing bonus feature and a
  host of options to tailor the game to any location.

  Match Games is today's perfect alternative in adult video skill games...


  MATCH GAMES...

  Unique and captivating graphics start the fun out right.
  Players score skills points as they match the amusing characters to winning color
  and number combinations. Play appeal stays high as new characters are introduced.

  "Wild" characters liven the action even more and build special bonus points, co-
  llected on a 5-way number match. The appeal is irresistible... Players stay hoo-
  ked in for lot more action (and more earnings) for you!.


  GAME THEMES:

  Match Games acknowledges every scoring combination by displaying its own special
  name keyed to each game theme.

  Every time 2 "Wild" characters pop up together, special bonus symbols appear,
  increasing bonus by 5 points.

  * "THE WHITE KNIGHT" features knights in armor with colored plumes and wild
                       'White Knights'.

  * "THE FROG POND" stars colorful and humorous frogs perched on top of mushrooms.

  * "WILD BULLS" is based around multi-colored sets of bulls with 2 wild 'Red Bulls'.

  * "PHARAOHS" features an Egyptian look with comical mummies as the wild characters.


  FEATURES:

  - Four game themes (operator selected).
  - "Wild" characters.
  - Bonus Feature, with color keyed graphics.
  - Adjustable game difficulty settings.
  - Adjustable play limit (1-10-20-50).
  - On screen bookkeeping.
  - Battery backup.
  - Hi quality 13" monitor.
  - Complete schematics.
  - Easy front access servicing.
  - All plywood construction.
  - Top drop coin entry.
  - Hi security cash box.


**********************************************************************************

  Hardware Notes:
  ---------------

  1x OSC = 18.000 MHz.
  1x CPU = Zilog Z80 (Z0840006PSC).
  4x 74LS253N (multiplexers).
  4x MM2114N-3 (4096-Bit Static RAM).
  2x SCM5101E (256x4 Static RAM).

  The PCB has a socket for two standard AA batteries


  MAIN BOARD CRT 100:
  .------------------------. .-------------------. .---------------------------.
  |                        | ||||||||||||||||||||| |           .------------.  |
  |                        '-'        J2         '-'          -|4.7 Ohm 10% |- |
  |                                                            '------------'  |
  |                                                                            |
  |         DSW (8)                .-+-.-+-.-+-.-+-.              LM340K-5     |
  |                                |   |   |   |   |              7805K        |
  |                                | 4x 4700uF 16V |    KBL005                 |
  |                                |  capacitors   |     8137                  |
  |                                |   |   |   |   |      AC                   |
  |                                '-+-'-+-'-+-'-+-'                           |
  |                                                                            |
  |                                                                            |
  |    74LS253N  74LS253N 74LS253N 74LS253                                     |
  |                                               .-------------.              |
  |                                               | 4700uF 35V  |   MC7812CT   |
  |                                              -|  capacitor  |-             |
  |                                               '-------------'              |
  |               .------------------.                                         |
  |               |    AA BATTERY    |                                         |
  |               |------------------|                                         |
  |               |    AA BATTERY    |                                   -     |
  | .-.           '------------------'                       74LS04N     -     |
  | | |                                                                  - J1  |
  | | |       74LS04N  74LS02N  74LS74N   74LS02N 74LS132N               -     |
  | | |                                                                  -     |
  | | |       74LS138N 74LS08N    7416N   74LS10N 74LS161N   74123N      -     |
  | | |                                                                  -     |
  | |J|       74LS00N  74LS11N  74LS161N 74LS157N 74LS161N    7416N            |
  | |4|                                                                        |
  | | |       74LS02N  74LS32N  74LS259N 74LS151N  74LS11N             LED     |
  | | |                                                18.000 OSC              |
  | | |                74LS367N 74LS161N 74LS161N  74LS04N    DM7404N          |
  | | |                                                                        |
  | | |        74LS245PC        .-------------.    74LS74N    LM3302N          |
  | | |                         |   Z80 CPU   |                                |
  | | |        74LS245PC        '-------------'   MM2114N-3  74LS174N 74LS166J |
  | | |                                                                .-----. |
  | '-'        74LS138N       74LS245PC 74LS245PC MM2114N-3  74LS157N  | U68 | |
  |                                                                    '-----' |
  |            SCM5101E                 74LS245PC MM2114N-3  74LS174N  .-----. |
  |                            .-----.                                 | U67 | |
  |            SCM5101E        |-U18-|  74LS157N  MM2114N-3  74LS138N  |-----| |
  |    .-----.  .-----.        |-----|                                 | U66 | |
  |    |-U16-|  |-U13-|        |-U19-|  74LS157N             74LS161N  '-----' |
  |    |-----|  |-----|        |-----|                                 .-----. |
  |    |-U15-|  |-U14-|        |-U17-|  74LS157N             74LS157N  | U65 | |
  |    '-----'  '-----'        '-----'                                 '-----' |
  '----------------------------------------------------------------------------'

  U13 = MLTI 0    U68 = CGM 0
  U14 = MLTI 1    U67 = CGM 1
  U15 = MLTI 2    U66 = CGM 2
  U16 = MLTI 3    U65 = CGM 3
  U17 = MLTI 4    U1 = Zilog Z0840006PSC Z80 CPU 9512 2K
  U18 = MLTI 5

  J4 = Conn to video I/O board
  J1 = Jumpers bank? (see multiplexed port)


  VIDEO I/O BOARD CRT 810:
                          .-------------------.
  .-----------------------|||||||||||||||||||||----------------------.
  |                                                                  |
  |                                                          LM380N  |
  |                                       DISCRETE                   |
  |                                       CIRCUITRY          MC1455P |
  |                                                                  |
  |                                                                  |
  |                                                           ML7805 |
  | SW 7407-N    SW 7407-N    SW 7407-N    SW 7407-N           +5V.  |
  |                                                                  |
  | 74LS259N     74LS259N     74LS259N                        MC7812 |
  |                                                            +12V. |
  |                                                                  |
  '----------------||||||||||||||||||||||||||||||||||----------------'
                   '--------------------------------'
                          To J4 on Main Board

  LM380N = 2.5W Audio Power Amplifier.
  MC1455P = Direct Replacement for NE555 Timers.

  (Audio IS discrete).

  4x 7407N (Buffer Gates Non-Inverting).
  4x 74LS259N (8-Bit Addressable Latches).


**********************************************************************************

  Dev Notes:
  ----------

  Additional work:

  * Full Inputs.
  * DIP Switches.
  * Simulated spark-watchdog circuitery.
  * Demuxed custom port.
  * NVRAM support.
  * CPU clock derived from #defined crystal.
  * 8000-8007 Output ports.
  * Coin related counters.
  * Sound components and trigger found at 8000-8003, bit2.
  * Full lamps support.
  * PCBs layouts & technical notes.

  Todo:

  - Writes to offset 0x0158 (so many!)

  - Still analyze 8000-8007 offset range remaining bits.
    These writes sounds like a BCD valueset.
    Maybe were intended formerly to send some data to 7seg display unit.

  - Color system (no bipolar PROMs in the system), needs a reference

  - Discrete sound.

**********************************************************************************/

#define MASTER_CLOCK    XTAL(18'000'000)

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include "mgames.lh"


namespace {

class mgames_state : public driver_device
{
public:
	mgames_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video(*this, "video"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void mgames(machine_config &config);

private:
	tilemap_t *m_tilemap = nullptr;

	uint8_t mixport_r();
	void outport0_w(uint8_t data);
	void outport1_w(uint8_t data);
	void outport2_w(uint8_t data);
	void outport3_w(uint8_t data);
	void outport4_w(uint8_t data);
	void outport5_w(uint8_t data);
	void outport6_w(uint8_t data);
	void outport7_w(uint8_t data);

	void mgames_palette(palette_device &palette) const;
	uint32_t screen_update_mgames(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tile_info);

	void main_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	uint8_t m_output[8]{};
	required_shared_ptr<uint8_t> m_video;
	int m_mixdata = 0;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<9> m_lamps;
};

TILE_GET_INFO_MEMBER( mgames_state::tile_info )
{
	uint8_t code = m_video[tile_index];
	uint8_t color = m_video[tile_index + 0x400] & 0x3f;

	tileinfo.set(0, code, color, 0);
}

void mgames_state::machine_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mgames_state::tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_lamps.resolve();
}

uint32_t mgames_state::screen_update_mgames(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void mgames_state::mgames_palette(palette_device &palette) const
{
	for (int i = 0; i < 64; i++)
	{
		palette.set_indirect_color(i, rgb_t(
			pal2bit((i >> 0) & 3),
			pal2bit((i >> 2) & 3),
			pal2bit((i >> 4) & 3)
		));
		palette.set_pen_indirect(i * 2 + 0, 0); // all tiles black background
		palette.set_pen_indirect(i * 2 + 1, i);
	}
}

uint8_t mgames_state::mixport_r()
{
/*  - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   Spark-Watchdog...
    ---- -x--   DIP Switch Bank #2.
    ---- x---   DIP Switch Bank #2.
    ---x ----   DIP Switch Bank #2.
    xxx- ----   Unknown.

    Spark-Watchdog... The system expect status changes on bit1, otherwise stop the hardware with an error message.
    The line seems to be tied to a clock. We can't use XORed status due to the nested checks.
    If you change the status *every* read, the HW stucks.
*/

	m_mixdata = (ioport("SW2")->read() & 0xfd) | (machine().rand() & 0x02);

	return m_mixdata;
}

/***** Port 0158 *****

    - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   Unknown.
    ---- -x--   Unknown.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

//void mgames_state::muxed_w(uint8_t data)
//{
//  popmessage("written : %02X %02X %02X %02X %02X %02X %02X %02X", data & 0x01, data & 0x02, data & 0x04, data & 0x08, data & 0x10, data & 0x20, data & 0x40, data & 0x80);
//}

/***** Port 8000 *****

    - bits -
    7654 3210
    ---- ---x   BET lamp.
    ---- --x-   HOLD1 lamp.
    ---- -x--   Sound component #1.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport0_w(uint8_t data)
{
	m_lamps[0] = BIT(data, 0);      /* Lamp 1 - BET */
	m_lamps[4] = BIT(data, 1);      /* Lamp 5 - HOLD 1 */

	m_output[0] = data;
	popmessage("outport0 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8001 *****

    - bits -
    7654 3210
    ---- ---x   DEAL lamp.
    ---- --x-   HOLD2 lamp.
    ---- -x--   Sound component #2.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport1_w(uint8_t data)
{
	m_lamps[1] = BIT(data, 0);      /* Lamp 2 - DEAL */
	m_lamps[5] = BIT(data, 1);      /* Lamp 6 - HOLD 2 */

	m_output[1] = data;
	popmessage("outport1 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8002 *****

    - bits -
    7654 3210
    ---- ---x   CANCEL lamp.
    ---- --x-   HOLD3 lamp.
    ---- -x--   Sound component #3.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport2_w(uint8_t data)
{
	m_lamps[2] = BIT(data, 0);      /* Lamp 3 - CANCEL */
	m_lamps[6] = BIT(data, 1);      /* Lamp 7 - HOLD 3 */

	m_output[2] = data;
	popmessage("outport2 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8003 *****

    - bits -
    7654 3210
    ---- ---x   Unknown lamp.
    ---- --x-   HOLD4 lamp.
    ---- -x--   Sound trigger.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport3_w(uint8_t data)
{
	m_lamps[3] = BIT(data, 0);      /* Lamp 4 - STAND */
	m_lamps[7] = BIT(data, 1);      /* Lamp 8 - HOLD 4 */

	m_output[3] = data;
	popmessage("outport3 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8004 *****

    - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   HOLD5 lamp.
    ---- -x--   Unknown.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport4_w(uint8_t data)
{
	m_lamps[8] = BIT(data, 1);      /* Lamp 9 - HOLD 5 */

	m_output[4] = data;
	popmessage("outport4 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8005 *****

    - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   Unknown.
    ---- -x--   Unknown.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport5_w(uint8_t data)
{
	m_output[5] = data;
	popmessage("outport5 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8006 *****

    - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   Payout pulse.
    ---- -x--   Unknown.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport6_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(1, data & 0x02);  /* Payout pulse */

	m_output[6] = data;
	popmessage("outport6 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}

/***** Port 8007 *****

    - bits -
    7654 3210
    ---- ---x   Unknown.
    ---- --x-   Coin pulse.
    ---- -x--   Unknown.
    ---- x---   Unknown.
    ---x ----   Unknown.
    xxx- ----   Unknown.
*/

void mgames_state::outport7_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x02);  /* Coin pulse */

	m_output[7] = data;
	popmessage("outport7 : %02X %02X %02X %02X %02X %02X %02X %02X", m_output[0], m_output[1], m_output[2], m_output[3], m_output[4], m_output[5], m_output[6], m_output[7]);
}


/********  Discrete Sound  ********

  There are 3 components plus a trigger:

  Component #1 = $8000, bit2.
  Component #2 = $8001, bit2.
  Component #3 = $8002, bit2.

  Trigger = $8003, bit2.

  All bits are inverted.


  Sound | 8000 | 8001 | 8002 | 8003
  ------+------+------+------+-----
   01   | bit2 | bit2 | bit2 | bit2
   02   | ---- | bit2 | bit2 | bit2
   03   | bit2 | ---- | bit2 | bit2
   04   | bit2 | bit2 | ---- | bit2
   05   | ---- | ---- | ---- | bit2


  We're tracing the discrete circuitry...

*/

void mgames_state::main_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
//  map(0x0158, 0x0158).w(FUNC(mgames_state::muxed_w));
	map(0x3800, 0x38ff).ram().share("nvram");   /* NVRAM = 2x SCM5101E */
	map(0x4000, 0x47ff).ram().share("video");   /* 4x MM2114N-3 */
	map(0x8000, 0x8000).portr("SW1");
	map(0x8001, 0x8001).r(FUNC(mgames_state::mixport_r)); /* DIP switch bank 2 + a sort of watchdog */
	map(0x8002, 0x8002).portr("IN1");
	map(0x8003, 0x8003).portr("IN2");
	map(0x8000, 0x8000).w(FUNC(mgames_state::outport0_w));
	map(0x8001, 0x8001).w(FUNC(mgames_state::outport1_w));
	map(0x8002, 0x8002).w(FUNC(mgames_state::outport2_w));
	map(0x8003, 0x8003).w(FUNC(mgames_state::outport3_w));
	map(0x8004, 0x8004).w(FUNC(mgames_state::outport4_w));
	map(0x8005, 0x8005).w(FUNC(mgames_state::outport5_w));
	map(0x8006, 0x8006).w(FUNC(mgames_state::outport6_w));
	map(0x8007, 0x8007).w(FUNC(mgames_state::outport7_w));
}


static INPUT_PORTS_START( mgames )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stand (Hold all Cards)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "20 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) // Yes, again...
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x09, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/20 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Game Select" )
	PORT_DIPSETTING(    0x00, "The White Knight" )
	PORT_DIPSETTING(    0x10, "Wild Bulls" )
	PORT_DIPSETTING(    0x20, "Pharaohs" )
	PORT_DIPSETTING(    0x30, "The Frog Pond" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximun Bet" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )      /* bit1 connected to a signal heartbeat */
	PORT_DIPNAME( 0x1c, 0x0c, "Main Percentage" )
	PORT_DIPSETTING(    0x18, "75%" )
	PORT_DIPSETTING(    0x14, "80%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPSETTING(    0x0c, "90%" )
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


static const gfx_layout gfx_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_mgames )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x1, 0, 64 )
GFXDECODE_END


void mgames_state::mgames(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);      /* 3 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &mgames_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mgames_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(mgames_state::screen_update_mgames));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mgames);
	PALETTE(config, m_palette, FUNC(mgames_state::mgames_palette), 128, 64);

	/* sound hardware */
	//  to do...
}


ROM_START( mgames )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "u13_mlt10_mk2716j.bin", 0x0000, 0x0800, CRC(ce2da863) SHA1(ddb921ac2fdd965138a91757843d3035144a7007) )
	ROM_LOAD( "u14_mlt11_mk2716j.bin", 0x0800, 0x0800, CRC(1382d166) SHA1(a8e7339f94d65b9540a8c16190a28ff0af48ccb4) )
	ROM_LOAD( "u15_mlt12_mk2716j.bin", 0x1000, 0x0800, CRC(eb12716c) SHA1(1589cb0aa180a3bfa5cb3da200b71f77c2191272) )
	ROM_LOAD( "u16_mlt13_mk2716j.bin", 0x1800, 0x0800, CRC(e2d80ff0) SHA1(f8aaa513f57da458ca89f999e30ea6b2d2ef41d3) )
	ROM_LOAD( "u17_mlt14_mk2716j.bin", 0x2000, 0x0800, CRC(36efcdf1) SHA1(d6fdc6abb1dbb5f812dc1c1ecb8d369bfcbf2b8a) )
	ROM_LOAD( "u18_mlt15_mk2716j.bin", 0x2800, 0x0800, CRC(7701c7df) SHA1(abf19f75367f926e49031e2fb4021172ebf176e1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "u68_cgm0_mk2716j.bin", 0x0000, 0x0800, CRC(3e4148e3) SHA1(bc5d173cc6ff17e0a7f06d36790e20d254be0377) )
	ROM_LOAD( "u67_cgm1_mk2716j.bin", 0x0800, 0x0800, CRC(c66c30ed) SHA1(9fb8fc669da37ff2e0379b023349ed314c8dcba4) )
	ROM_LOAD( "u66_cgm2_mk2716j.bin", 0x1000, 0x0800, CRC(3b6b98d3) SHA1(9fc1d9d61d67ad696750c21ef9a19968ddb0a9e1) )
	ROM_LOAD( "u65_cgm3_mk2716j.bin", 0x1800, 0x0800, CRC(d61ae9d1) SHA1(219123518999fc925397db4f442ac444dfddffbe) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME    PARENT  MACHINE  INPUT   CLASS         INIT        ROT   COMPANY  FULLNAME       FLAGS...                                 LAYOUT  */
GAMEL( 1981, mgames, 0,      mgames,  mgames, mgames_state, empty_init, ROT0, "Merit", "Match Games", MACHINE_WRONG_COLORS | MACHINE_NO_SOUND, layout_mgames )
