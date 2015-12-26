// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, David Haywood
/*********************************************************************************

  Merit Industries Match Games (1981)
  -----------------------------------

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
  with brillant graphics, challenging play action, an enticing bonus feature and a
  host of options to tailor the game to any location.

  Match Games is today's perfect alternative in adult video skill games...


  MATCH GAMES...

  Unique and captivating graphics start the fun out right.
  Players score skills points as they match the amusing characters to winning color
  and number combinations. Play appeal stays high as new characters are introduced.

  "Wild" characters liven the action even more and build special bonus points, co-
  llected on a 5-way number match. The appeal is irresistable... Players stay hoo-
  ked in for lot more action (and more earnings) for you!.


  GAME THEMES:

  Match Games aknowledges every scoring combination by displaying its own special
  name keyed to each game theme.

  Every time 2 "Wild" characters pop up together, special bonus symbols appear,
  increasing bonus by 5 points.

  * "THE WHITE KNIGHT" features knights in armor with colores plumes and wild
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
  1x CPU = Zilog Z80 (Z0840006PSC / 9512 / 2K).
  4x 74LS253N (multiplexers).
  4x MM2114N-3 (4096-Bit Static RAM).
  2x SCM5101E (256x4 Static RAM).

  The PCB has a socket for two standard AA batteries


  MAIN BOARD:
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


  VIDEO I/O BOARD CRT810:
                          .-------------------.
                          |||||||||||||||||||||
  .-----------------------'                   '----------------------.
  |                                                                  |
  |                                                          LM380N  |
  |                                                                  |
  |                                                          MC1455P |
  |                                                                  |
  |                                                                  |
  | SW 7407-N    SW 7407-N    SW 7407-N    SW 7407-N                 |
  | 21430 7301   21430 7301   21430 7301   21430 7301                |
  |                                                                  |
  | 74LS259N     74LS259N     74LS259N     74LS259N                  |
  |                                                                  |
  '----------------.                                .----------------'
                   ||||||||||||||||||||||||||||||||||
                   '--------------------------------'
                           To J4 on Main Board

  LM380N = 2.5W Audio Power Amplifier.
  MC1455P = Direct Replacement for NE555 Timers.

  (Audio seems to be discrete).

  4x 7407N (Buffer Gates Non-Inverting).
  4x 74LS259N (8-Bit Addressable Latches).

  4x 7301 (HDSP?)(RED Seven Segment Displays).

  +---------+  Pin | Description
  |    A    |  ----+------------
  |   ---   |   01 | Anode (4).
  | F|   |B |   02 | Cathode F.
  |   -G-   |   03 | Cathode G.
  | E|   |C |   04 | Cathode E.
  |   ---   |   05 | Cathode D.
  |    D .DP|   06 | Anode (4).
  +---------+   07 | Cathode DP.
                08 | Cathode C.
                09 | Cathode B.
                10 | Cathode A.


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


**********************************************************************************/

#define MASTER_CLOCK    XTAL_18MHz

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/dac.h"
#include "machine/nvram.h"
#include "mgames.lh"


class mgames_state : public driver_device
{
public:
	mgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video(*this, "video"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_output[8];
	required_shared_ptr<UINT8> m_video;
	int m_mixdata;
	DECLARE_READ8_MEMBER(mixport_r);
	DECLARE_WRITE8_MEMBER(outport0_w);
	DECLARE_WRITE8_MEMBER(outport1_w);
	DECLARE_WRITE8_MEMBER(outport2_w);
	DECLARE_WRITE8_MEMBER(outport3_w);
	DECLARE_WRITE8_MEMBER(outport4_w);
	DECLARE_WRITE8_MEMBER(outport5_w);
	DECLARE_WRITE8_MEMBER(outport6_w);
	DECLARE_WRITE8_MEMBER(outport7_w);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mgames);
	UINT32 screen_update_mgames(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void mgames_state::video_start()
{
}

UINT32 mgames_state::screen_update_mgames(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x;
	int count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	count = 0;
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 dat = m_video[count];
			UINT16 col = m_video[count+0x400] & 0x7f;
			gfx->opaque(bitmap,cliprect,dat,col,0,0,x*16,y*16);
			count++;
		}

	}
	return 0;
}

PALETTE_INIT_MEMBER(mgames_state, mgames)
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		rgb_t color;

		if (i & 0x01)
			color = rgb_t(pal2bit((i & 0x6) >> 1),pal2bit((i & 0x18) >> 3),pal2bit((i & 0x60) >> 5));
		else
			color = rgb_t::black;

		palette.set_pen_color(i, color);
	}
}

READ8_MEMBER(mgames_state::mixport_r)
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

//WRITE8_MEMBER(mgames_state::muxed_w)
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

WRITE8_MEMBER(mgames_state::outport0_w)
{
	output_set_lamp_value(1, (data & 1));           /* Lamp 1 - BET */
	output_set_lamp_value(5, (data >> 1) & 1);      /* Lamp 5 - HOLD 1 */

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

WRITE8_MEMBER(mgames_state::outport1_w)
{
	output_set_lamp_value(2, (data & 1));           /* Lamp 2 - DEAL */
	output_set_lamp_value(6, (data >> 1) & 1);      /* Lamp 6 - HOLD 2 */

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

WRITE8_MEMBER(mgames_state::outport2_w)
{
	output_set_lamp_value(3, (data & 1));           /* Lamp 3 - CANCEL */
	output_set_lamp_value(7, (data >> 1) & 1);      /* Lamp 7 - HOLD 3 */

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

WRITE8_MEMBER(mgames_state::outport3_w)
{
	output_set_lamp_value(4, (data & 1));           /* Lamp 4 - STAND */
	output_set_lamp_value(8, (data >> 1) & 1);      /* Lamp 8 - HOLD 4 */

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

WRITE8_MEMBER(mgames_state::outport4_w)
{
	output_set_lamp_value(9, (data >> 1) & 1);      /* Lamp 9 - HOLD 5 */

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

WRITE8_MEMBER(mgames_state::outport5_w)
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

WRITE8_MEMBER(mgames_state::outport6_w)
{
	coin_counter_w(machine(), 1, data & 0x02);  /* Payout pulse */

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

WRITE8_MEMBER(mgames_state::outport7_w)
{
	coin_counter_w(machine(), 0, data & 0x02);  /* Coin pulse */

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


WRITE8_MEMBER(mgames_state::sound_w)
//{
//  m_dac->write_unsigned8(data);
//}
*/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, mgames_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
//  AM_RANGE(0x0158, 0x0158) AM_WRITE (muxed_w)
	AM_RANGE(0x3800, 0x38ff) AM_RAM AM_SHARE("nvram")   /* NVRAM = 2x SCM5101E */
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("video")   /* 4x MM2114N-3 */
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("SW1")
	AM_RANGE(0x8001, 0x8001) AM_READ(mixport_r) /* DIP switch bank 2 + a sort of watchdog */
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1")
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2")
	AM_RANGE(0x8000, 0x8000) AM_WRITE(outport0_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITE(outport1_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(outport2_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(outport3_w)
	AM_RANGE(0x8004, 0x8004) AM_WRITE(outport4_w)
	AM_RANGE(0x8005, 0x8005) AM_WRITE(outport5_w)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(outport6_w)
	AM_RANGE(0x8007, 0x8007) AM_WRITE(outport7_w)
ADDRESS_MAP_END


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
//  PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) --> damn check... you can't set 2 different bits pointing to the same coinage.
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
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


static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16},
	16*16
};

static GFXDECODE_START( mgames )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END


static MACHINE_CONFIG_START( mgames, mgames_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/6)      /* 3 MHz? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mgames_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(mgames_state, screen_update_mgames)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mgames)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(mgames_state, mgames)

	/* sound hardware */
//  MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_DAC_ADD("dac")
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


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


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT   STATE          INIT   ROT    COMPANY  FULLNAME      FLAGS...                           LAYOUT  */
GAMEL( 1981, mgames,   0,      mgames,   mgames, driver_device, 0,     ROT0, "Merit", "Match Games", MACHINE_WRONG_COLORS | MACHINE_NO_SOUND, layout_mgames )
