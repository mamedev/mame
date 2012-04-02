/*********************************************************************************

  Merit Industries Multi-Poker (1981)
  -----------------------------------

  Driver by Angelo Salese, David Haywood & Roberto Fresca.

  Maybe the first Merit videopoker.
  The system has 4 different themed videopokers selectable through DIP switches:

  * "The Frog Pond"
  * "Pharaohs"
  * "Wild Bulls"
  * "The White Knight"

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
   _________________________   ___________________   __________________________
  |                         |_|||||||||||||||||||||_|                          |
  |                                  J2                       -|4.7 Ohm 10% |- |
  |                                                            |____________|  |
  |                                                                            |
  |         DSW (8)                 _|___|___|___|_               LM340K-5     |
  |                                |   |   |   |   |              7805K        |
  |                                | 4x4700uF 16V  |    KBL005                 |
  |                                |   |   |   |   |     8137                  |
  |                                |   |   |   |   |      AC                   |
  |                                |___|___|___|___|                           |
  |                                  |   |   |   |                             |
  |                                                                            |
  |    74LS253N  74LS253N 74LS253N 74LS253                                     |
  |                                                _____________               |
  |                                               | 4700uF 35V  |   MC7812CT   |
  |                                              -|  Capacitor  |-             |
  |                                               |_____________|              |
  |                                                                            |
  |                                                                            |
  |                _____________________                                       |
  |               |    B A T T E R Y    |                                -     |
  |  _            |_____________________|                    74LS04N     -     |
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
  | | |        74LS245PC             _________     74LS74N    LM3302N          |
  | | |                             | Z80 CPU |                                |
  | | |        74LS245PC            |___U1____|   MM2114N-3  74LS174N 74LS166J |
  | | |                                                                 _____  |
  | |_|        74LS138N       74LS245PC 74LS245PC MM2114N-3  74LS157N  |_U68_| |
  |                                                                            |
  |            SCM5101E                 74LS245PC MM2114N-3  74LS174N   _____  |
  |                             _____                                  |_U67_| |
  |            SCM5101E        |_U18_|  74LS157N  MM2114N-3  74LS138N   _____  |
  |     _____    _____          _____                                  |_U66_| |
  |    |_U16_|  |_U13_|        |_U19_|  74LS157N             74LS161N          |
  |     _____    _____          _____                                   _____  |
  |    |_U15_|  |_U14_|        |_U17_|  74LS157N             74LS157N  |_U65_| |
  |                                                                            |
  |____________________________________________________________________________|


  U13 = MLTI 0    U68 = CGM 0
  U14 = MLTI 1    U67 = CGM 1
  U15 = MLTI 2    U66 = CGM 2
  U16 = MLTI 3    U65 = CGM 3
  U17 = MLTI 4    U1 = Zilog Z0840006PSC Z80 CPU 9512 2K
  U18 = MLTI 5

  J4 = Conn to video I/O board
  J1 = Jumpers bank? (see multiplexed port)


  VIDEO I/O BOARD CRT810:
                          ____________________
   _______________________|||||||||||||||||||||______________________
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
  |________________                                  ________________|
                   ||||||||||||||||||||||||||||||||||
                   ---------------------------------
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

#define MASTER_CLOCK	XTAL_18MHz

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/dac.h"
#include "machine/nvram.h"
#include "mpoker.lh"


class mpoker_state : public driver_device
{
public:
	mpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_output[8];
	UINT8* m_video;
	int m_mixdata;
};


static VIDEO_START(mpoker)
{

}

static SCREEN_UPDATE_IND16(mpoker)
{
	mpoker_state *state = screen.machine().driver_data<mpoker_state>();
	int y,x;
	int count;
	const gfx_element *gfx = screen.machine().gfx[0];

	count = 0;
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 dat = state->m_video[count];
			UINT16 col = state->m_video[count+0x400] & 0x7f;
			drawgfx_opaque(bitmap,cliprect,gfx,dat,col,0,0,x*16,y*16);
			count++;
		}

	}
	return 0;
}

static PALETTE_INIT(mpoker)
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		rgb_t color;

		if (i & 0x01)
			color = MAKE_RGB(pal2bit((i & 0x6) >> 1),pal2bit((i & 0x18) >> 3),pal2bit((i & 0x60) >> 5));
		else
			color = RGB_BLACK;

		palette_set_color(machine, i, color);
	}
}

static READ8_HANDLER( mixport_r )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
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

	state->m_mixdata = (input_port_read(space->machine(), "SW2") & 0xfd) | (space->machine().rand() & 0x02);

	return state->m_mixdata;
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

//static WRITE8_HANDLER( muxed_w )
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

static WRITE8_HANDLER( outport0_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	output_set_lamp_value(1, (data & 1));			/* Lamp 1 - BET */
	output_set_lamp_value(5, (data >> 1) & 1);		/* Lamp 5 - HOLD 1 */

	state->m_output[0] = data;
	popmessage("outport0 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport1_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	output_set_lamp_value(2, (data & 1));			/* Lamp 2 - DEAL */
	output_set_lamp_value(6, (data >> 1) & 1);		/* Lamp 6 - HOLD 2 */

	state->m_output[1] = data;
	popmessage("outport1 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport2_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	output_set_lamp_value(3, (data & 1));			/* Lamp 3 - CANCEL */
	output_set_lamp_value(7, (data >> 1) & 1);		/* Lamp 7 - HOLD 3 */

	state->m_output[2] = data;
	popmessage("outport2 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport3_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	output_set_lamp_value(4, (data & 1));			/* Lamp 4 - STAND */
	output_set_lamp_value(8, (data >> 1) & 1);		/* Lamp 8 - HOLD 4 */

	state->m_output[3] = data;
	popmessage("outport3 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport4_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	output_set_lamp_value(9, (data >> 1) & 1);		/* Lamp 9 - HOLD 5 */

	state->m_output[4] = data;
	popmessage("outport4 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport5_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	state->m_output[5] = data;
	popmessage("outport5 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport6_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	coin_counter_w(space->machine(), 1, data & 0x02);	/* Payout pulse */

	state->m_output[6] = data;
	popmessage("outport6 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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

static WRITE8_HANDLER( outport7_w )
{
	mpoker_state *state = space->machine().driver_data<mpoker_state>();
	coin_counter_w(space->machine(), 0, data & 0x02);	/* Coin pulse */

	state->m_output[7] = data;
	popmessage("outport7 : %02X %02X %02X %02X %02X %02X %02X %02X", state->m_output[0], state->m_output[1], state->m_output[2], state->m_output[3], state->m_output[4], state->m_output[5], state->m_output[6], state->m_output[7]);
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


//static WRITE8_HANDLER( sound_w )
//{
//  dac_data_w(space->machine().device("dac"), data);
//}
*/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, mpoker_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
//  AM_RANGE(0x0158, 0x0158) AM_WRITE (muxed_w)
	AM_RANGE(0x3800, 0x38ff) AM_RAM AM_SHARE("nvram")	/* NVRAM = 2x SCM5101E */
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(m_video)	/* 4x MM2114N-3 */
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("SW1")
	AM_RANGE(0x8001, 0x8001) AM_READ_LEGACY(mixport_r) /* DIP switch bank 2 + a sort of watchdog */
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1")
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2")
	AM_RANGE(0x8000, 0x8000) AM_WRITE_LEGACY(outport0_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITE_LEGACY(outport1_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE_LEGACY(outport2_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE_LEGACY(outport3_w)
	AM_RANGE(0x8004, 0x8004) AM_WRITE_LEGACY(outport4_w)
	AM_RANGE(0x8005, 0x8005) AM_WRITE_LEGACY(outport5_w)
	AM_RANGE(0x8006, 0x8006) AM_WRITE_LEGACY(outport6_w)
	AM_RANGE(0x8007, 0x8007) AM_WRITE_LEGACY(outport7_w)

ADDRESS_MAP_END

static INPUT_PORTS_START( mpoker )
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )		/* bit1 connected to a signal heartbeat */
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

static GFXDECODE_START( mpoker )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END

static MACHINE_CONFIG_START( mpoker, mpoker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/6)		 /* 3 MHz? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(mpoker)

	MCFG_GFXDECODE(mpoker)
	MCFG_PALETTE_LENGTH(0x200)

	MCFG_PALETTE_INIT(mpoker)
	MCFG_VIDEO_START(mpoker)

	/* sound hardware */
//  MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("dac", DAC, 0)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( mpoker )
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

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT   ROT    COMPANY  FULLNAME      FLAGS...                           LAYOUT  */
GAMEL( 1981, mpoker,   0,      mpoker,   mpoker,   0,     ROT0, "Merit", "Multi-Poker", GAME_WRONG_COLORS | GAME_NO_SOUND, layout_mpoker )
